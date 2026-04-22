#include "alarm/alarm_manager.h"

#include <QAbstractEventDispatcher>
#include <algorithm>

#include "log/log_event.h"
#include "log/log_interface.h"
#include "runtime/machine_runtime.h"
#include "settings/settings_manager.h"

namespace {
int metricDecimals(const QString &metricKey)
{
    if (metricKey == "pressure") {
        return 2;
    }

    return 1;
}

double metricEpsilon(const QString &metricKey)
{
    if (metricKey == "pressure") {
        return 0.01;
    }

    return 0.05;
}

QString formatDurationSeconds(qint64 totalSeconds)
{
    const qint64 safeSeconds = std::max<qint64>(0, totalSeconds);
    const qint64 minutes = safeSeconds / 60;
    const qint64 seconds = safeSeconds % 60;
    if (minutes <= 0) {
        return QString("%1s").arg(seconds);
    }

    return QString("%1m %2s").arg(minutes).arg(seconds, 2, 10, QLatin1Char('0'));
}
} // namespace

AlarmManager::AlarmManager(LogInterface &logInterface,
                           SettingsManager &settings,
                           MachineRuntime &runtime,
                           QObject *parent,
                           int recoveryNoticeDurationMs)
    : QObject(parent)
    , m_logInterface(&logInterface)
    , m_settings(&settings)
    , m_runtime(&runtime)
{
    Q_ASSERT(m_logInterface);
    Q_ASSERT(m_settings);
    Q_ASSERT(m_runtime);

    m_lastObservedTemperature = m_runtime->temperature();
    m_lastObservedPressure = m_runtime->pressure();
    m_hasObservedValues = true;

    m_recoveryTimer.setSingleShot(true);
    m_recoveryTimer.setInterval(recoveryNoticeDurationMs);

    connect(m_settings, &SettingsManager::thresholdsChanged, this, [this]() {
        if (m_lifecycle != AlarmLifecycle::FaultLatched
            && m_lifecycle != AlarmLifecycle::ResetRequested) {
            evaluateAlarm();
        }
    });

    connect(m_runtime, &MachineRuntime::resetAlarmState, this, &AlarmManager::evaluateAlarm);
    connect(m_runtime, &MachineRuntime::faultResetRequested, this, &AlarmManager::enterResetRequested);
    connect(m_runtime, &MachineRuntime::faultResetCompleted, this, &AlarmManager::enterRecoveryNotice);
    connect(m_runtime, &MachineRuntime::evaluateAlarm, this, &AlarmManager::evaluateAlarm);

    connect(m_runtime, &MachineRuntime::stateChanged, this, [this]() {
        if (m_lifecycle == AlarmLifecycle::RecoveryNotice
            && m_runtime->state() != MachineRuntime::State::Idle) {
            m_recoveryTimer.stop();
            m_lifecycle = AlarmLifecycle::Normal;
            evaluateAlarm();
        }
    });

    connect(&m_recoveryTimer, &QTimer::timeout, this, [this]() {
        if (m_lifecycle == AlarmLifecycle::RecoveryNotice) {
            m_lifecycle = AlarmLifecycle::Normal;
            evaluateAlarm();
        }
    });

    connect(this, &AlarmManager::warningEntered, m_runtime, [this](const QString &) {
        m_runtime->recordHistoryMarker("warning", "Warning", "#f59e0b");
    });

    connect(this, &AlarmManager::warningCleared, m_runtime, [this]() {
        m_runtime->recordHistoryMarker("clear", "Clear", "#cbd5e1");
    });

    connect(this, &AlarmManager::faultEntered, m_runtime, [this](const QString &) {
        m_runtime->recordHistoryMarker("fault", "Fault", "#ef4444");
    });

    connect(this, &AlarmManager::warningEntered, this, [this](const QString &message) {
        appendLog(LogEvent{
            .level = "WARNING",
            .source = "alarm",
            .eventType = "alarm.warning.entered",
            .message = message,
        });
    });

    connect(this, &AlarmManager::warningCleared, this, [this]() {
        appendLog(LogEvent{
            .level = "INFO",
            .source = "alarm",
            .eventType = "alarm.warning.cleared",
            .message = "Warning cleared",
        });
    });

    connect(this, &AlarmManager::faultEntered, this, [this](const QString &message) {
        appendLog(LogEvent{
            .level = "FAULT",
            .source = "alarm",
            .eventType = "alarm.fault.entered",
            .message = message,
        });
    });

    connect(this, &AlarmManager::recoveryEntered, this, [this]() {
        appendLog(LogEvent{
            .level = "INFO",
            .source = "alarm",
            .eventType = "alarm.recovery.notice.entered",
            .message = "Fault cleared and machine returned to idle",
        });
    });
}

QString AlarmManager::alarmText() const
{
    return m_headline;
}

QString AlarmManager::headline() const
{
    return m_headline;
}

QString AlarmManager::detail() const
{
    return m_detail;
}

QString AlarmManager::operatorHint() const
{
    return m_operatorHint;
}

QString AlarmManager::stateLabel() const
{
    return m_stateLabel;
}

QString AlarmManager::lifecycleState() const
{
    switch (m_lifecycle) {
    case AlarmLifecycle::Normal:
        return "Normal";
    case AlarmLifecycle::WarningActive:
        return "WarningActive";
    case AlarmLifecycle::FaultLatched:
        return "FaultLatched";
    case AlarmLifecycle::ResetRequested:
        return "ResetRequested";
    case AlarmLifecycle::RecoveryNotice:
        return "RecoveryNotice";
    }

    return "Normal";
}

bool AlarmManager::hasWarning() const
{
    return m_lifecycle == AlarmLifecycle::WarningActive;
}

bool AlarmManager::isFault() const
{
    return m_lifecycle == AlarmLifecycle::FaultLatched
           || m_lifecycle == AlarmLifecycle::ResetRequested;
}

bool AlarmManager::recoveryActive() const
{
    return m_lifecycle == AlarmLifecycle::RecoveryNotice;
}

QString AlarmManager::activeMetric() const
{
    return m_activeMetric;
}

void AlarmManager::evaluateAlarm()
{
    if (m_lifecycle == AlarmLifecycle::FaultLatched
        || m_lifecycle == AlarmLifecycle::ResetRequested
        || m_lifecycle == AlarmLifecycle::RecoveryNotice) {
        return;
    }

    const Snapshot &snapshot = m_settings->snapshot();
    const double temperature = m_runtime->temperature();
    const double pressure = m_runtime->pressure();

    const bool temperatureFault = temperature >= snapshot.faultTemperature;
    const bool pressureFault = pressure >= snapshot.faultPressure;
    if (temperatureFault || pressureFault) {
        const QString faultMetric = selectFaultMetric(temperatureFault, pressureFault);
        const MetricContext context = faultMetric == "pressure"
            ? makeMetricContext("pressure",
                                pressure,
                                snapshot.warningPressure,
                                snapshot.faultPressure)
            : makeMetricContext("temperature",
                                temperature,
                                snapshot.warningTemperature,
                                snapshot.faultTemperature);
        enterFault(context);
        updateObservedValues(temperature, pressure);
        return;
    }

    const bool temperatureWarning = temperature >= snapshot.warningTemperature;
    const bool pressureWarning = pressure >= snapshot.warningPressure;
    const bool temperatureWarningEdge = temperatureWarning && !m_temperatureWarningActive;
    const bool pressureWarningEdge = pressureWarning && !m_pressureWarningActive;
    const QDateTime now = QDateTime::currentDateTime();

    if (temperatureWarningEdge) {
        m_temperatureWarningStartedAt = now;
    } else if (!temperatureWarning) {
        m_temperatureWarningStartedAt = QDateTime{};
    }

    if (pressureWarningEdge) {
        m_pressureWarningStartedAt = now;
    } else if (!pressureWarning) {
        m_pressureWarningStartedAt = QDateTime{};
    }

    m_temperatureWarningActive = temperatureWarning;
    m_pressureWarningActive = pressureWarning;

    if (temperatureWarning || pressureWarning) {
        const QString warningMetric = selectWarningMetric(temperatureWarning,
                                                          pressureWarning,
                                                          temperatureWarningEdge,
                                                          pressureWarningEdge);
        const bool emitEntrySignal = (warningMetric == "temperature" && temperatureWarningEdge)
                                     || (warningMetric == "pressure" && pressureWarningEdge);
        const MetricContext context = warningMetric == "pressure"
            ? makeMetricContext("pressure",
                                pressure,
                                snapshot.warningPressure,
                                snapshot.faultPressure)
            : makeMetricContext("temperature",
                                temperature,
                                snapshot.warningTemperature,
                                snapshot.faultTemperature);
        enterWarning(context, emitEntrySignal);
        updateObservedValues(temperature, pressure);
        return;
    }

    enterNormal();
    updateObservedValues(temperature, pressure);
}

void AlarmManager::enterNormal()
{
    const bool wasWarning = m_lifecycle == AlarmLifecycle::WarningActive;

    m_recoveryTimer.stop();
    clearWarningTracking();
    m_lastFaultMetric.clear();

    setPresentation(AlarmLifecycle::Normal,
                    "System normal",
                    "Temperature and pressure are within configured limits.",
                    "",
                    "Normal",
                    "");

    if (wasWarning) {
        emit warningCleared();
    }
}

void AlarmManager::enterWarning(const MetricContext &context, bool emitEntrySignal)
{
    setPresentation(AlarmLifecycle::WarningActive,
                    QString("%1 warning").arg(metricLabel(context.key)),
                    formatWarningDetail(context, warningStartedAtForMetric(context.key)),
                    "Monitor trend and reduce load before the fault limit is reached.",
                    "Warning",
                    context.key);

    if (emitEntrySignal) {
        emit warningEntered(m_detail);
    }
}

void AlarmManager::enterFault(const MetricContext &context)
{
    if (m_lifecycle == AlarmLifecycle::FaultLatched) {
        return;
    }

    const QDateTime warningStartedAt = warningStartedAtForMetric(context.key);
    const bool warningWasActive = warningStartedAt.isValid();
    const qint64 warningDurationSeconds = warningWasActive
        ? warningStartedAt.secsTo(QDateTime::currentDateTime())
        : 0;
    const QString triggerState = m_runtime->status();

    m_recoveryTimer.stop();
    clearWarningTracking();
    m_lastFaultMetric = context.key;

    setPresentation(AlarmLifecycle::FaultLatched,
                    QString("Over-%1 fault").arg(metricLabel(context.key).toLower()),
                    formatFaultDetail(context,
                                      triggerState,
                                      warningWasActive,
                                      warningDurationSeconds),
                    "Machine stopped in safe state. Inspect cause, then press Reset Fault.",
                    "Fault",
                    context.key);

    m_runtime->enterFault();
    emit faultEntered(m_detail);
}

void AlarmManager::enterResetRequested()
{
    const QString metric = m_lastFaultMetric.isEmpty() ? m_activeMetric : m_lastFaultMetric;
    setPresentation(AlarmLifecycle::ResetRequested,
                    "Fault reset requested",
                    "Waiting for the backend to return the machine from Fault to Idle.",
                    "Do not start the machine until recovery is confirmed.",
                    "Resetting",
                    metric);
}

void AlarmManager::enterRecoveryNotice()
{
    const QString metric = m_lastFaultMetric.isEmpty() ? m_activeMetric : m_lastFaultMetric;

    setPresentation(AlarmLifecycle::RecoveryNotice,
                    "Fault cleared",
                    "Machine returned to Idle after reset. Telemetry restored to safe idle values.",
                    "System is ready for a new start sequence.",
                    "Recovered",
                    metric);

    clearWarningTracking();
    m_lastFaultMetric.clear();
    if (QAbstractEventDispatcher::instance(thread()) != nullptr) {
        m_recoveryTimer.start();
    }
    emit recoveryEntered();
}

void AlarmManager::setPresentation(AlarmLifecycle lifecycle,
                                   const QString &headline,
                                   const QString &detail,
                                   const QString &operatorHint,
                                   const QString &stateLabel,
                                   const QString &activeMetric)
{
    const bool changed = m_lifecycle != lifecycle || m_headline != headline || m_detail != detail
        || m_operatorHint != operatorHint || m_stateLabel != stateLabel
        || m_activeMetric != activeMetric;
    if (!changed) {
        return;
    }

    m_lifecycle = lifecycle;
    m_headline = headline;
    m_detail = detail;
    m_operatorHint = operatorHint;
    m_stateLabel = stateLabel;
    m_activeMetric = activeMetric;
    emit alarmChanged();
}

AlarmManager::MetricContext AlarmManager::makeMetricContext(const QString &metricKey,
                                                           double currentValue,
                                                           double warningThreshold,
                                                           double faultThreshold) const
{
    return MetricContext{
        .key = metricKey,
        .currentValue = currentValue,
        .warningThreshold = warningThreshold,
        .faultThreshold = faultThreshold,
        .trend = trendForMetric(metricKey, currentValue),
    };
}

QString AlarmManager::selectWarningMetric(bool temperatureWarning,
                                          bool pressureWarning,
                                          bool temperatureWarningEdge,
                                          bool pressureWarningEdge) const
{
    if (temperatureWarningEdge && !pressureWarningEdge) {
        return "temperature";
    }

    if (pressureWarningEdge && !temperatureWarningEdge) {
        return "pressure";
    }

    if (m_activeMetric == "temperature" && temperatureWarning) {
        return "temperature";
    }

    if (m_activeMetric == "pressure" && pressureWarning) {
        return "pressure";
    }

    if (temperatureWarning && !pressureWarning) {
        return "temperature";
    }

    if (pressureWarning && !temperatureWarning) {
        return "pressure";
    }

    return "temperature";
}

QString AlarmManager::selectFaultMetric(bool temperatureFault, bool pressureFault) const
{
    if (m_activeMetric == "temperature" && temperatureFault) {
        return "temperature";
    }

    if (m_activeMetric == "pressure" && pressureFault) {
        return "pressure";
    }

    if (temperatureFault) {
        return "temperature";
    }

    return "pressure";
}

QString AlarmManager::metricLabel(const QString &metricKey) const
{
    if (metricKey == "pressure") {
        return "Pressure";
    }

    return "Temperature";
}

QString AlarmManager::formatMetricValue(const QString &metricKey, double value) const
{
    const QString unit = metricKey == "pressure" ? "bar" : "C";
    return QString("%1%2").arg(QString::number(value, 'f', metricDecimals(metricKey)), unit);
}

QString AlarmManager::formatWarningDetail(const MetricContext &context,
                                         const QDateTime &warningStartedAt) const
{
    const double remainingToFault = std::max(0.0, context.faultThreshold - context.currentValue);
    QString detail = QString("%1 %2 exceeded warning limit %3; %4 remaining to fault limit %5; trend %6.")
                         .arg(metricLabel(context.key),
                              formatMetricValue(context.key, context.currentValue),
                              formatMetricValue(context.key, context.warningThreshold),
                              formatMetricValue(context.key, remainingToFault),
                              formatMetricValue(context.key, context.faultThreshold),
                              context.trend);

    if (warningStartedAt.isValid()) {
        detail += QString(" Warning active for %1.")
                      .arg(formatDurationSeconds(warningStartedAt.secsTo(QDateTime::currentDateTime())));
    }

    return detail;
}

QString AlarmManager::formatFaultDetail(const MetricContext &context,
                                        const QString &triggerState,
                                        bool warningWasActive,
                                        qint64 warningDurationSeconds) const
{
    QString detail = QString("%1 %2 exceeded fault limit %3 while %4.")
                         .arg(metricLabel(context.key),
                              formatMetricValue(context.key, context.currentValue),
                              formatMetricValue(context.key, context.faultThreshold),
                              triggerState);

    if (warningWasActive) {
        detail += QString(" Warning was active for %1 before trip.")
                      .arg(formatDurationSeconds(warningDurationSeconds));
    } else {
        detail += " Fault triggered without a prior warning hold.";
    }

    return detail;
}

QString AlarmManager::trendForMetric(const QString &metricKey, double currentValue) const
{
    if (!m_hasObservedValues) {
        return "steady";
    }

    const double previousValue = metricKey == "pressure" ? m_lastObservedPressure
                                                         : m_lastObservedTemperature;
    const double epsilon = metricEpsilon(metricKey);
    if (currentValue > previousValue + epsilon) {
        return "rising";
    }
    if (currentValue < previousValue - epsilon) {
        return "falling";
    }

    return "steady";
}

QDateTime AlarmManager::warningStartedAtForMetric(const QString &metricKey) const
{
    return metricKey == "pressure" ? m_pressureWarningStartedAt : m_temperatureWarningStartedAt;
}

void AlarmManager::clearWarningTracking()
{
    m_temperatureWarningActive = false;
    m_pressureWarningActive = false;
    m_temperatureWarningStartedAt = QDateTime{};
    m_pressureWarningStartedAt = QDateTime{};
}

void AlarmManager::updateObservedValues(double temperature, double pressure)
{
    m_lastObservedTemperature = temperature;
    m_lastObservedPressure = pressure;
    m_hasObservedValues = true;
}

void AlarmManager::appendLog(const LogEvent &event)
{
    m_logInterface->appendLog(event);
}
