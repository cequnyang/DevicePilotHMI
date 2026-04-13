#pragma once

#include <QObject>
#include <QPointer>
#include <QString>
#include <qqmlintegration.h>

#include "settings/settings_defined.h"

class LogInterface;

using Settings::Snapshot;
class SettingsManager : public QObject
{
    Q_OBJECT

public:
    explicit SettingsManager(LogInterface &logInterface, QObject *parent = nullptr);

signals:
    void thresholdsChanged();
    void updateIntervalMsChanged();

public:
    int updateIntervalMs() const;
    const Snapshot &snapshot() const;

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
    void appendLog(const QString &level, const QString &message);

private:
    QPointer<LogInterface> m_logInterface{nullptr};
    Snapshot m_snapshot{Settings::Default::kWarningTemperature,
                        Settings::Default::kFaultTemperature,
                        Settings::Default::kWarningPressure,
                        Settings::Default::kFaultPressure,
                        Settings::Default::kUpdateIntervalMs};
};
