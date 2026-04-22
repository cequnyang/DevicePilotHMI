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

bool SettingsManager::showTimestamp() const
{
    return m_logViewPreferences.showTimestamp;
}

bool SettingsManager::showSource() const
{
    return m_logViewPreferences.showSource;
}

bool SettingsManager::showLevel() const
{
    return m_logViewPreferences.showLevel;
}

void SettingsManager::setShowTimestamp(bool value)
{
    if (m_logViewPreferences.showTimestamp == value) {
        return;
    }

    Settings::LogViewPreferences candidate = m_logViewPreferences;
    candidate.showTimestamp = value;

    const auto [persist, persistReason] = persistConfiguration(m_snapshot, candidate);
    if (!persist) {
        appendLog(makePersistenceEvent(
            "CONFIG",
            "persistence.log_view.persist.failed",
            QString("Failed to persist log column visibility for timestamp (%1)")
                .arg(persistReason)));
        return;
    }

    m_logViewPreferences = candidate;
    emit showTimestampChanged();
}

void SettingsManager::setShowSource(bool value)
{
    if (m_logViewPreferences.showSource == value) {
        return;
    }

    Settings::LogViewPreferences candidate = m_logViewPreferences;
    candidate.showSource = value;

    const auto [persist, persistReason] = persistConfiguration(m_snapshot, candidate);
    if (!persist) {
        appendLog(makePersistenceEvent(
            "CONFIG",
            "persistence.log_view.persist.failed",
            QString("Failed to persist log column visibility for source (%1)")
                .arg(persistReason)));
        return;
    }

    m_logViewPreferences = candidate;
    emit showSourceChanged();
}

void SettingsManager::setShowLevel(bool value)
{
    if (m_logViewPreferences.showLevel == value) {
        return;
    }

    Settings::LogViewPreferences candidate = m_logViewPreferences;
    candidate.showLevel = value;

    const auto [persist, persistReason] = persistConfiguration(m_snapshot, candidate);
    if (!persist) {
        appendLog(makePersistenceEvent(
            "CONFIG",
            "persistence.log_view.persist.failed",
            QString("Failed to persist log column visibility for level (%1)").arg(persistReason)));
        return;
    }

    m_logViewPreferences = candidate;
    emit showLevelChanged();
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

    const auto [persist, persistReason] = persistConfiguration(candidate, m_logViewPreferences);
    if (!persist) {
        appendLog(makePersistenceEvent(
            "CONFIG",
            "persistence.persist.failed",
            QString("Failed to persist settings configuration (%1)").arg(persistReason)));
        return {false, persistReason};
    }

    const Snapshot oldSnapshot = m_snapshot;
    m_snapshot = candidate;
    emitSnapshotChanges(oldSnapshot, m_snapshot);
    return {true, {}};
}

void SettingsManager::load()
{
    const auto [loaded, repaired, loadReason] = Settings::Store::loadConfig();

    m_snapshot = loaded.snapshot;
    m_logViewPreferences = loaded.logViewPreferences;
    if (!repaired) {
        appendLog(makePersistenceEvent(
            "INFO",
            "persistence.load.succeeded",
            "Settings configuration loaded from settings.json."));
        return;
    }

    const auto [persist, persistReason] = persistConfiguration(m_snapshot, m_logViewPreferences);
    if (!persist) {
        appendLog(makePersistenceEvent(
            "CONFIG",
            "persistence.repair.failed",
            QString("Recovered settings configuration but failed to rewrite settings.json (%1)")
                .arg(persistReason)));
        return;
    }

    appendLog(makePersistenceEvent(
        "CONFIG",
        "persistence.load.repaired",
        loadReason.isEmpty()
            ? "Settings configuration was repaired and rewritten."
            : QString("Settings configuration was repaired and rewritten (%1)")
                  .arg(loadReason)));
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

Settings::Store::PersistResult SettingsManager::persistConfiguration(
    const Snapshot &snapshot,
    const Settings::LogViewPreferences &prefs)
{
    return Settings::Store::persistConfig(Settings::PersistedConfig{snapshot, prefs});
}

void SettingsManager::appendLog(const LogEvent &event)
{
    m_logInterface->appendLog(event);
}
