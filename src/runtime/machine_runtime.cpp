#include "runtime/machine_runtime.h"

#include <algorithm>

#include "log/log_interface.h"
#include "settings/settings_manager.h"

MachineRuntime::MachineRuntime(LogInterface &logInterface,
                               SettingsManager &settings,
                               QObject *parent)
    : QObject(parent)
    , m_logInterface(&logInterface)
    , m_settings(&settings)
{
    Q_ASSERT(m_logInterface);
    Q_ASSERT(m_settings);

    m_updateTimer.setInterval(m_settings->updateIntervalMs());
    connect(&m_updateTimer, &QTimer::timeout, this, &MachineRuntime::updateSimulation);

    m_transitionTimer.setSingleShot(true);
    connect(&m_transitionTimer, &QTimer::timeout, this, &MachineRuntime::onTransitionTimeout);

    connect(m_settings, &SettingsManager::updateIntervalMsChanged, this, [this]() {
        m_updateTimer.setInterval(m_settings->updateIntervalMs());
        appendLog("CONFIG",
                  QString("updateIntervalMs applied: %1 ms").arg(m_settings->updateIntervalMs()));
    });
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
    m_pendingTransition = PendingTransition::FinishStart;
    setState(State::Starting);
    m_transitionTimer.start(5000);
}

void MachineRuntime::stop()
{
    if (!canStop()) {
        return;
    }

    appendLog("INFO", "Stop requested");
    m_pendingTransition = PendingTransition::FinishStop;
    setState(State::Stopping);
    m_transitionTimer.start(1200);
}

void MachineRuntime::resetFault()
{
    if (!canResetFault()) {
        return;
    }

    appendLog("INFO", "Fault reset requested");

    m_pendingTransition = PendingTransition::None;
    resetMeasurementsToIdle();
    emit resetAlarmState();
    setState(State::Idle);

    appendLog("INFO", "Fault reset completed");
}

void MachineRuntime::updateSimulation()
{
    if (m_state != State::Running) {
        return;
    }

    m_temperature += 1.6;
    m_pressure += 2.4;
    m_speed = std::min(3600, m_speed + 120);

    emit temperatureChanged();
    emit pressureChanged();
    emit speedChanged();
    emit evaluateAlarm();
}

void MachineRuntime::onTransitionTimeout()
{
    switch (m_pendingTransition) {
    case PendingTransition::FinishStart:
        m_pendingTransition = PendingTransition::None;
        m_speed = 800;
        emit speedChanged();
        setState(State::Running);
        m_updateTimer.start();
        appendLog("INFO", "Transition to Running completed");
        emit evaluateAlarm();
        break;

    case PendingTransition::FinishStop:
        m_pendingTransition = PendingTransition::None;
        m_updateTimer.stop();
        resetMeasurementsToIdle();
        emit resetAlarmState();
        setState(State::Idle);
        appendLog("INFO", "Transition to Idle completed");
        break;

    case PendingTransition::None:
        break;
    }
}

void MachineRuntime::onEnterFault()
{
    m_updateTimer.stop();
    m_transitionTimer.stop();
    m_pendingTransition = PendingTransition::None;
    if (m_speed != 0) {
        m_speed = 0;
        emit speedChanged();
    }
    setState(State::Fault);
}

void MachineRuntime::setState(State newState)
{
    if (m_state == newState) {
        return;
    }

    m_state = newState;
    emit statusChanged();
    emit stateChanged();
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

void MachineRuntime::resetMeasurementsToIdle()
{
    if (m_temperature != RuntimeInit::kTemperature) {
        m_temperature = RuntimeInit::kTemperature;
        emit temperatureChanged();
    }
    if (m_pressure != RuntimeInit::kPressure) {
        m_pressure = RuntimeInit::kPressure;
        emit pressureChanged();
    }
    if (m_speed != RuntimeInit::kSpeed) {
        m_speed = RuntimeInit::kSpeed;
        emit speedChanged();
    }
}

void MachineRuntime::appendLog(const QString &level, const QString &message)
{
    m_logInterface->appendLog(level, message);
}
