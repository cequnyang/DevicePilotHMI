#pragma once

#include <QObject>
#include <QPointer>
#include <qqmlintegration.h>

#include "settings/settings_defined.h"
#include "settings/settings_draft.h"

class LogInterface;
class SettingsApplyService;

using Settings::Snapshot;
class SettingsSession : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("SettingsSession is created in C++ and injected into the root QML object.")

    Q_PROPERTY(SettingsDraft *draft READ draft CONSTANT)
    Q_PROPERTY(bool applyEnabled READ applyEnabled NOTIFY applyStateChanged)
    Q_PROPERTY(QString applyRestrictionReason READ applyRestrictionReason NOTIFY applyStateChanged)
    Q_PROPERTY(int committedWarningTemperature READ committedWarningTemperature NOTIFY
                   committedSettingsChanged)
    Q_PROPERTY(int committedFaultTemperature READ committedFaultTemperature NOTIFY
                   committedSettingsChanged)
    Q_PROPERTY(int committedWarningPressure READ committedWarningPressure NOTIFY
                   committedSettingsChanged)
    Q_PROPERTY(int committedFaultPressure READ committedFaultPressure NOTIFY
                   committedSettingsChanged)

public:
    explicit SettingsSession(LogInterface &logInterface,
                             SettingsManager &settingsManager,
                             SettingsApplyService &settingsApplyService,
                             QObject *parent = nullptr);

    SettingsDraft *draft() const;
    bool applyEnabled() const;
    QString applyRestrictionReason() const;
    int committedWarningTemperature() const;
    int committedFaultTemperature() const;
    int committedWarningPressure() const;
    int committedFaultPressure() const;

    Q_INVOKABLE bool apply();
    Q_INVOKABLE void reload();

signals:
    void applyStateChanged();
    void committedSettingsChanged();

private:
    const Snapshot &snapshotFromDraft() const;
    void appendLog(const QString &level, const QString &message);

private:
    QPointer<LogInterface> m_logInterface{nullptr};
    QPointer<SettingsManager> m_manager{nullptr};
    QPointer<SettingsApplyService> m_applyService{nullptr};
    SettingsDraft *m_draft{nullptr};
};
