#include "settings/settings_manager.h"

#include <QObject>
#include <QPointer>
#include <QString>

#include "log/log_event.h"
#include "log/log_interface.h"
#include "settings/settings_file_store.h"
#include "settings/settings_validation.h"

namespace {
LogEvent makeSettingsEvent(const QString &level,
                           const QString &eventType,
                           const QString &message)
{
    return LogEvent{
        .level = level,
        .source = "settings",
        .eventType = eventType,
        .message = message,
    };
}

LogEvent makePersistenceEvent(const QString &level,
                              const QString &eventType,
                              const QString &message)
{
    return LogEvent{
        .level = level,
        .source = "persistence",
        .eventType = eventType,
        .message = message,
    };
}
} // namespace

SettingsManager::SettingsManager(LogInterface &logInterface, QObject *parent)
    : QObject(parent)
    , m_logInterface(&logInterface)
{
    Q_ASSERT(m_logInterface);

    load();
}

int SettingsManager::updateIntervalMs() const
{
    return m_snapshot.updateIntervalMs;
}

const Snapshot &SettingsManager::snapshot() const
{
    return m_snapshot;
}

SettingsManager::ApplyResult SettingsManager::applySnapshot(const Snapshot &candidate)
{
    const auto [validation, validationReason] = Settings::validateSnapshot(candidate);
    if (!validation) {
        appendLog(makeSettingsEvent(
            "CONFIG",
            "settings.snapshot.validation.failed",
            QString("Failed to validate settings snapshot (%1)").arg(validationReason)));
        return {false, validationReason};
    }

    if (candidate == m_snapshot) {
        return {true, {}};
    }

    const auto [persist, persistReason] = Settings::Store::persistSnapshot(candidate);
    if (!persist) {
        appendLog(makePersistenceEvent(
            "CONFIG",
            "persistence.persist.failed",
            QString("Failed to persist settings snapshot (%1)").arg(persistReason)));
        return {false, persistReason};
    }

    const Snapshot oldSnapshot = m_snapshot;
    m_snapshot = candidate;
    emitSnapshotChanges(oldSnapshot, m_snapshot);
    return {true, {}};
}

void SettingsManager::load()
{
    const auto [loaded, repaired, loadReason] = Settings::Store::loadSnapshot();

    m_snapshot = loaded;
    if (!repaired) {
        appendLog(makePersistenceEvent("INFO",
                                       "persistence.load.succeeded",
                                       "Settings snapshot loaded from settings.json."));
        return;
    }

    const auto [persist, persistReason] = Settings::Store::persistSnapshot(m_snapshot);
    if (!persist) {
        appendLog(makePersistenceEvent(
            "CONFIG",
            "persistence.repair.failed",
            QString("Recovered settings snapshot but failed to rewrite settings.json (%1)")
                .arg(persistReason)));
        return;
    }

    appendLog(makePersistenceEvent(
        "CONFIG",
        "persistence.load.repaired",
        loadReason.isEmpty()
            ? "Settings snapshot was repaired and rewritten."
            : QString("Settings snapshot was repaired and rewritten (%1)").arg(loadReason)));
}

void SettingsManager::emitSnapshotChanges(const Snapshot &oldSnapshot, const Snapshot &newSnapshot)
{
    bool thresholdGroupChanged = false;
    bool updateIntervalChanged = false;
    const auto changeAndLog = [this](bool &change,
                                     const QString &eventType,
                                     const QString &key,
                                     const auto &oldValue,
                                     const auto &newValue) -> void {
        if (oldValue == newValue) {
            return;
        }
        appendLog(makeSettingsEvent(
            "CONFIG",
            eventType,
            QString("%1 changed from %2 to %3").arg(key).arg(oldValue).arg(newValue)));
        change = true;
    };

    changeAndLog(thresholdGroupChanged,
                 "settings.threshold.changed",
                 "Warning Temperature",
                 oldSnapshot.warningTemperature,
                 newSnapshot.warningTemperature);
    changeAndLog(thresholdGroupChanged,
                 "settings.threshold.changed",
                 "Fault Temperature",
                 oldSnapshot.faultTemperature,
                 newSnapshot.faultTemperature);
    changeAndLog(thresholdGroupChanged,
                 "settings.threshold.changed",
                 "Warning Pressure",
                 oldSnapshot.warningPressure,
                 newSnapshot.warningPressure);
    changeAndLog(thresholdGroupChanged,
                 "settings.threshold.changed",
                 "Fault Pressure",
                 oldSnapshot.faultPressure,
                 newSnapshot.faultPressure);
    changeAndLog(updateIntervalChanged,
                 "settings.update_interval.changed",
                 "Interval Time",
                 oldSnapshot.updateIntervalMs,
                 newSnapshot.updateIntervalMs);

    if (thresholdGroupChanged) {
        emit thresholdsChanged();
    }

    if (updateIntervalChanged) {
        emit updateIntervalMsChanged();
    }
}

void SettingsManager::appendLog(const LogEvent &event)
{
    m_logInterface->appendLog(event);
}
