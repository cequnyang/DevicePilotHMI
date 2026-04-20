#include "runtime/machine_runtime.h"

#include <QMetaType>
#include <algorithm>
#include <chrono>
#include <format>
#include <string>

#include "backend/machine_backend.h"
#include "log/log_interface.h"

MachineRuntime::MachineRuntime(LogInterface &logInterface,
                               MachineBackend &backend,
                               QObject *parent)
    : QObject(parent)
    , m_backend(&backend)
    , m_logInterface(&logInterface)
{
    qRegisterMetaType<MachineState>("MachineState");
    qRegisterMetaType<TelemetryFrame>("TelemetryFrame");

    Q_ASSERT(m_backend);
    Q_ASSERT(m_logInterface);

    m_stateContextTimer.setInterval(1000);
    connect(&m_stateContextTimer, &QTimer::timeout, this, [this]() { emit stateContextChanged(); });
    m_stateContextTimer.start();

    connect(m_backend,
            &MachineBackend::telemetryReceived,
            this,
            &MachineRuntime::onTelemetryReceived);

    connect(m_backend, &MachineBackend::stateReported, this, &MachineRuntime::onStateReported);
}

QString MachineRuntime::status() const
{
    return stateToString(m_state);
}

double MachineRuntime::temperature() const
{
    return m_temperature;
}

double MachineRuntime::pressure() const
{
    return m_pressure;
}

int MachineRuntime::speed() const
{
    return m_speed;
}

bool MachineRuntime::canStart() const
{
    return m_state == State::Idle;
}

bool MachineRuntime::canStop() const
{
    return m_state == State::Starting || m_state == State::Running;
}

bool MachineRuntime::canResetFault() const
{
    return m_state == State::Fault;
}

QString MachineRuntime::startDisabledReason() const
{
    if (canStart()) {
        return "";
    }

    switch (m_state) {
    case State::Starting:
        return "Start request already in progress.";
    case State::Running:
        return "Machine is already running.";
    case State::Stopping:
        return "Start is available again after the machine returns to Idle.";
    case State::Fault:
        return "Start unavailable while a fault is active.";
    case State::Idle:
        return "";
    }
    return "";
}

QString MachineRuntime::stopDisabledReason() const
{
    if (canStop()) {
        return "";
    }

    switch (m_state) {
    case State::Idle:
        return "Stop is available only during Starting or Running.";
    case State::Stopping:
        return "Stop request already in progress.";
    case State::Fault:
        return "Stop unavailable while the machine is in Fault.";
    case State::Starting:
    case State::Running:
        return "";
    }
    return "";
}

QString MachineRuntime::resetDisabledReason() const
{
    if (canResetFault()) {
        return "";
    }

    return "Reset Fault is available only in Fault state.";
}

QVariantList MachineRuntime::temperatureHistory() const
{
    return m_temperatureHistory;
}

QVariantList MachineRuntime::pressureHistory() const
{
    return m_pressureHistory;
}

QVariantList MachineRuntime::speedHistory() const
{
    return m_speedHistory;
}

QVariantList MachineRuntime::historyMarkers() const
{
    return m_historyMarkers;
}

int MachineRuntime::historyStartSampleIndex() const
{
    return m_historyStartSampleIndex;
}

namespace {
std::string formatDuration(long long totalSeconds)
{
    auto hms = std::chrono::hh_mm_ss{std::chrono::seconds{totalSeconds}};
    return std::format("{}h {}m {}s",
                       hms.hours().count(),
                       hms.minutes().count(),
                       hms.seconds().count());
}
} // namespace

QString MachineRuntime::currentStateDurationText() const
{
    return QString::fromStdString(formatDuration(currentStateDurationSeconds()));
}

qint64 MachineRuntime::currentStateDurationSeconds() const
{
    if (!m_stateEnteredAt.isValid()) {
        return 0;
    }

    return std::max<qint64>(0, m_stateEnteredAt.secsTo(QDateTime::currentDateTime()));
}

QDateTime MachineRuntime::lastTransitionTime() const
{
    return m_lastTransitionTime;
}

QString MachineRuntime::lastTransitionTimeText() const
{
    if (!m_lastTransitionTime.isValid()) {
        return "Not observed yet";
    }

    return m_lastTransitionTime.toString("yyyy-MM-dd hh:mm:ss");
}

MachineRuntime::State MachineRuntime::state() const
{
    return m_state;
}

void MachineRuntime::start()
{
    if (!canStart()) {
        return;
    }

    appendLog("INFO", "Start requested");
    recordHistoryMarker("start", "Start", "#22c55e");
    m_backend->requestStart();
}

void MachineRuntime::stop()
{
    if (!canStop()) {
        return;
    }

    appendLog("INFO", "Stop requested");
    recordHistoryMarker("stop", "Stop", "#94a3b8");
    m_backend->requestStop();
}

void MachineRuntime::resetFault()
{
    if (!canResetFault()) {
        return;
    }

    appendLog("INFO", "Fault reset requested");
    recordHistoryMarker("reset", "Reset", "#a78bfa");
    m_faultResetPending = true;
    m_backend->requestResetFault();
}

namespace {
void appendSample(QVariantList &history, double value, int capacity)
{
    history.append(QVariant::fromValue(value));
    while (history.size() > capacity) {
        history.removeFirst();
    }
}
int markerSampleIndex(const QVariant &markerValue)
{
    const QVariantMap marker = markerValue.toMap();
    return marker.value("sampleIndex", -1).toInt();
}
} // namespace

void MachineRuntime::onTelemetryReceived(TelemetryFrame frame)
{
    bool telemetryChanged = false;

    if (m_temperature != frame.temperature) {
        m_temperature = frame.temperature;
        emit temperatureChanged();
        telemetryChanged = true;
    }

    if (m_pressure != frame.pressure) {
        m_pressure = frame.pressure;
        emit pressureChanged();
        telemetryChanged = true;
    }

    if (m_speed != frame.speed) {
        m_speed = frame.speed;
        emit speedChanged();
        telemetryChanged = true;
    }

    m_lastSampleIndex = m_nextSampleIndex++;
    appendSample(m_temperatureHistory, frame.temperature, kHistoryCapacity);
    appendSample(m_pressureHistory, frame.pressure, kHistoryCapacity);
    appendSample(m_speedHistory, frame.speed, kHistoryCapacity);

    const int historySize = static_cast<int>(m_temperatureHistory.size());
    m_historyStartSampleIndex = std::max(0, m_nextSampleIndex - historySize);
    trimHistoryMarkers();

    if (telemetryChanged && m_state == State::Running) {
        emit evaluateAlarm();
    }
    emit historyChanged();
}

void MachineRuntime::onStateReported(MachineState state)
{
    if (m_state == State::Fault) {
        if (state == State::Idle && !m_faultResetPending) {
            return;
        }

        if (state != State::Fault && state != State::Idle) {
            return;
        }
    }

    if (m_state == state) {
        return;
    }

    const State previousState = m_state;
    m_state = state;
    markStateTransition(QDateTime::currentDateTime());

    emit statusChanged();
    emit stateChanged();
    emit stateContextChanged();

    if (m_state == State::Fault) {
        m_faultResetPending = false;
    }

    if (previousState == State::Starting && m_state == State::Running) {
        appendLog("INFO", "Transition to Running completed");
        recordHistoryMarker("running", "Running", "#38bdf8");
        emit evaluateAlarm();
        return;
    }

    if (previousState == State::Stopping && m_state == State::Idle) {
        appendLog("INFO", "Transition to Idle completed");
        recordHistoryMarker("idle", "Idle", "#cbd5e1");
        emit resetAlarmState();
        return;
    }

    if (previousState == State::Fault && m_state == State::Idle) {
        m_faultResetPending = false;
        appendLog("INFO", "Fault reset completed");
        recordHistoryMarker("idle", "Idle", "#cbd5e1");
        emit resetAlarmState();
    }
}

void MachineRuntime::enterFault()
{
    if (m_state == State::Fault) {
        return;
    }

    m_state = State::Fault;
    m_faultResetPending = false;
    markStateTransition(QDateTime::currentDateTime());

    emit statusChanged();
    emit stateChanged();
    emit stateContextChanged();

    if (m_speed != RuntimeInit::kSpeed) {
        m_speed = RuntimeInit::kSpeed;
        emit speedChanged();
    }

    m_backend->requestSafeShutdown();
}

void MachineRuntime::appendLog(const QString &level, const QString &message)
{
    m_logInterface->appendLog(level, message);
}

QString MachineRuntime::stateToString(State state) const
{
    switch (state) {
    case State::Idle:
        return "Idle";
    case State::Starting:
        return "Starting";
    case State::Running:
        return "Running";
    case State::Stopping:
        return "Stopping";
    case State::Fault:
        return "Fault";
    }
    return "Unknown";
}

void MachineRuntime::trimHistoryMarkers()
{
    while (!m_historyMarkers.isEmpty()) {
        const int sampleIndex = markerSampleIndex(m_historyMarkers.first());
        if (sampleIndex >= m_historyStartSampleIndex) {
            break;
        }
        m_historyMarkers.removeFirst();
    }
}

void MachineRuntime::markStateTransition(const QDateTime &timestamp)
{
    m_stateEnteredAt = timestamp;
    m_lastTransitionTime = timestamp;
}

void MachineRuntime::recordHistoryMarker(const QString &kind,
                                         const QString &label,
                                         const QString &color)
{
    QVariantMap marker;
    marker["sampleIndex"] = std::max(0, m_lastSampleIndex >= 0 ? m_lastSampleIndex : m_nextSampleIndex);
    marker["kind"] = kind;
    marker["label"] = label;
    marker["color"] = color;

    m_historyMarkers.append(marker);
    trimHistoryMarkers();
    emit historyChanged();
}
