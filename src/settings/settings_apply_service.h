#pragma once

#include <QObject>
#include <QPointer>
#include <QString>
#include <qqmlintegration.h>

#include "settings/settings_defined.h"

class LogInterface;
class LogEvent;
class SettingsManager;
class MachineRuntime;

using Settings::Snapshot;
struct SettingsApplyAnalysis
{
    bool allowed{false};
    QString reason;
    bool changesThresholds{false};
    bool changesUpdateInterval{false};
};

class SettingsApplyService : public QObject
{
    Q_OBJECT

public:
    explicit SettingsApplyService(LogInterface &logInterface,
                                  SettingsManager &settings,
                                  MachineRuntime &machineRuntime,
                                  QObject *parent = nullptr);

    SettingsApplyAnalysis analyzeSettingsApply(const Snapshot &candidate) const;
    bool applySettings(const Snapshot &candidate);
    QString settingsApplyRestrictionReason(const Snapshot &candidate) const;

signals:
    void policyContextChanged();

private:
    void appendLog(const LogEvent &event);

private:
    QPointer<LogInterface> m_logInterface{nullptr};
    QPointer<SettingsManager> m_settings{nullptr};
    QPointer<MachineRuntime> m_machineRuntime{nullptr};
};