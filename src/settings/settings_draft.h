#pragma once

#include <QObject>
#include <QString>
#include <qqmlintegration.h>

#include "settings/settings_defined.h"

class SettingsManager;

using Settings::Snapshot;
class SettingsDraft : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int warningTemperature READ warningTemperature WRITE setWarningTemperature NOTIFY
                   warningTemperatureChanged)
    Q_PROPERTY(int faultTemperature READ faultTemperature WRITE setFaultTemperature NOTIFY
                   faultTemperatureChanged)
    Q_PROPERTY(int warningPressure READ warningPressure WRITE setWarningPressure NOTIFY
                   warningPressureChanged)
    Q_PROPERTY(
        int faultPressure READ faultPressure WRITE setFaultPressure NOTIFY faultPressureChanged)
    Q_PROPERTY(int updateIntervalMs READ updateIntervalMs WRITE setUpdateIntervalMs NOTIFY
                   updateIntervalMsChanged)

    Q_PROPERTY(QString validationMessage READ validationMessage NOTIFY stateChanged)
    Q_PROPERTY(bool dirty READ dirty NOTIFY stateChanged)
    Q_PROPERTY(bool canApply READ canApply NOTIFY stateChanged)

public:
    explicit SettingsDraft(QObject *parent = nullptr);

    int warningTemperature() const;
    int faultTemperature() const;
    int warningPressure() const;
    int faultPressure() const;
    int updateIntervalMs() const;
    QString validationMessage() const;
    bool dirty() const;
    bool canApply() const;

    void setWarningTemperature(int value);
    void setFaultTemperature(int value);
    void setWarningPressure(int value);
    void setFaultPressure(int value);
    void setUpdateIntervalMs(int value);

    Q_INVOKABLE void resetDraftToDefaults();

signals:
    void warningTemperatureChanged();
    void faultTemperatureChanged();
    void warningPressureChanged();
    void faultPressureChanged();
    void updateIntervalMsChanged();
    void stateChanged();

public:
    void loadFrom(SettingsManager *settings);
    void loadSnapshot(const Snapshot &snapshot);
    const Snapshot &snapshot() const;
    bool valid() const;

private:
    void emitDraftChange(const Snapshot &old);
    void revalidate();

private:
    Snapshot m_snapshot{Settings::Default::kWarningTemperature,
                        Settings::Default::kFaultTemperature,
                        Settings::Default::kWarningPressure,
                        Settings::Default::kFaultPressure,
                        Settings::Default::kUpdateIntervalMs};

    Snapshot m_originalSnapshot{Settings::Default::kWarningTemperature,
                                Settings::Default::kFaultTemperature,
                                Settings::Default::kWarningPressure,
                                Settings::Default::kFaultPressure,
                                Settings::Default::kUpdateIntervalMs};

    bool m_valid{true};
    bool m_invalidValueSetAttempted{false};
    QString m_validationMessage;
};
