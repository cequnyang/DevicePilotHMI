#include "alarm/alarm_manager.h"

#include "log/log_event.h"
#include "log/log_interface.h"
#include "runtime/machine_runtime.h"
#include "settings/settings_manager.h"

AlarmManager::AlarmManager(LogInterface &logInterface,
                           SettingsManager &settings,
                           MachineRuntime &runtime,
                           QObject *parent)
    : QObject(parent)
    , m_logInterface(&logInterface)
    , m_settings(&settings)
    , m_runtime(&runtime)
{
    Q_ASSERT(m_logInterface);
    Q_ASSERT(m_settings);
    Q_ASSERT(m_runtime);

    connect(m_settings, &SettingsManager::thresholdsChanged, this, [this]() {
        if (m_alarmLevel != AlarmLevel::Fault) {
            evaluateAlarm();
        }
    });

    connect(m_runtime, &MachineRuntime::resetAlarmState, this, [this]() {
        const bool hadActiveAlarm = m_alarmLevel != AlarmLevel::Normal;
        m_alarmLevel = AlarmLevel::Normal;
        m_alarmText = "System normal";
        m_activeMetric.clear();
        emit alarmChanged();
        if (hadActiveAlarm) {
            emit alarmCleared();
        }
    });

    connect(m_runtime, &MachineRuntime::evaluateAlarm, this, &AlarmManager::evaluateAlarm);

    connect(this, &AlarmManager::warningEntered, m_runtime, [this](const QString &) {
        m_runtime->recordHistoryMarker("warning", "Warning", "#f59e0b");
    });

    connect(this, &AlarmManager::faultEntered, m_runtime, [this](const QString &) {
        m_runtime->recordHistoryMarker("fault", "Fault", "#ef4444");
    });

    connect(this, &AlarmManager::alarmCleared, m_runtime, [this]() {
        m_runtime->recordHistoryMarker("clear", "Clear", "#cbd5e1");
    });

    connect(this, &AlarmManager::warningEntered, this, [this](const QString &text) {
        appendLog(LogEvent{
            .level = "WARNING",
            .source = "alarm",
            .eventType = "alarm.warning.entered",
            .message = text,
        });
    });

    connect(this, &AlarmManager::faultEntered, this, [this](const QString &text) {
        appendLog(LogEvent{
            .level = "FAULT",
            .source = "alarm",
            .eventType = "alarm.fault.entered",
            .message = text,
        });
    });

    connect(this, &AlarmManager::alarmCleared, this, [this]() {
        appendLog(LogEvent{
            .level = "INFO",
            .source = "alarm",
            .eventType = "alarm.cleared",
            .message = "Alarm cleared",
        });
    });
}

QString AlarmManager::alarmText() const
{
    return m_alarmText;
}

bool AlarmManager::hasWarning() const
{
    return m_alarmLevel == AlarmLevel::Warning;
}

bool AlarmManager::isFault() const
{
    return m_alarmLevel == AlarmLevel::Fault;
}

QString AlarmManager::activeMetric() const
{
    return m_activeMetric;
}

void AlarmManager::evaluateAlarm()
{
    if (isFault()) {
        return;
    }

    const Snapshot &snapShot = m_settings->snapshot();
    const auto temperature = m_runtime->temperature();
    if (temperature >= snapShot.faultTemperature) {
        enterFault("Temperature exceeded fault threshold", "temperature");
        return;
    }

    const auto pressure = m_runtime->pressure();
    if (pressure >= snapShot.faultPressure) {
        enterFault("Pressure exceeded fault threshold", "pressure");
        return;
    }

    AlarmLevel newLevel = AlarmLevel::Normal;
    QString newAlarmText = "System normal";
    QString newActiveMetric;
    if (temperature >= snapShot.warningTemperature) {
        newLevel = AlarmLevel::Warning;
        newAlarmText = "Temperature exceeded warning threshold";
        newActiveMetric = "temperature";
    } else if (pressure >= snapShot.warningPressure) {
        newLevel = AlarmLevel::Warning;
        newAlarmText = "Pressure exceeded warning threshold";
        newActiveMetric = "pressure";
    }

    if (m_alarmLevel != newLevel || m_alarmText != newAlarmText || m_activeMetric != newActiveMetric) {
        if (newLevel == AlarmLevel::Warning && m_alarmLevel != AlarmLevel::Warning) {
            emit warningEntered(newAlarmText);
        } else if (newLevel == AlarmLevel::Normal && m_alarmLevel != AlarmLevel::Normal) {
            emit alarmCleared();
        }

        m_alarmLevel = newLevel;
        m_alarmText = newAlarmText;
        m_activeMetric = newActiveMetric;
        emit alarmChanged();
    }
}

void AlarmManager::enterFault(const QString &reason, const QString &metric)
{
    if (isFault()) {
        return;
    }

    m_alarmLevel = AlarmLevel::Fault;
    m_alarmText = reason;
    m_activeMetric = metric;
    m_runtime->enterFault();
    emit alarmChanged();
    emit faultEntered(reason);
}

void AlarmManager::appendLog(const LogEvent &event)
{
    m_logInterface->appendLog(event);
}
