#pragma once

#include <QObject>
#include <QPointer>
#include <QString>

#include "settings/settings_defined.h"
#include "settings/settings_file_store.h"

class LogInterface;
struct LogEvent;

using Settings::Snapshot;
class SettingsManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool showTimestamp READ showTimestamp WRITE setShowTimestamp NOTIFY
                   showTimestampChanged)
    Q_PROPERTY(bool showSource READ showSource WRITE setShowSource NOTIFY showSourceChanged)
    Q_PROPERTY(bool showLevel READ showLevel WRITE setShowLevel NOTIFY showLevelChanged)

public:
    explicit SettingsManager(LogInterface &logInterface, QObject *parent = nullptr);

signals:
    void thresholdsChanged();
    void updateIntervalMsChanged();
    void showTimestampChanged();
    void showSourceChanged();
    void showLevelChanged();

public:
    int updateIntervalMs() const;
    const Snapshot &snapshot() const;
    bool showTimestamp() const;
    bool showSource() const;
    bool showLevel() const;

    void setShowTimestamp(bool value);
    void setShowSource(bool value);
    void setShowLevel(bool value);

    struct ApplyResult
    {
        bool ok{false};
        QString reason;

        explicit operator bool() const { return ok; }
    };
    ApplyResult applySnapshot(const Snapshot &candidate);

private:
    void load();
    void emitSnapshotChanges(const Snapshot &oldSnapshot, const Snapshot &newSnapshot);
    Settings::Store::PersistResult persistConfiguration(const Snapshot &snapshot,
                                                        const Settings::LogViewPreferences &prefs);
    void appendLog(const LogEvent &event);

private:
    QPointer<LogInterface> m_logInterface{nullptr};
    Snapshot m_snapshot{Settings::Default::kWarningTemperature,
                        Settings::Default::kFaultTemperature,
                        Settings::Default::kWarningPressure,
                        Settings::Default::kFaultPressure,
                        Settings::Default::kUpdateIntervalMs};
    Settings::LogViewPreferences m_logViewPreferences{Settings::defaultLogViewPreferences()};
};
