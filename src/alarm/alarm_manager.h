#pragma once

#include <QObject>
#include <QPointer>
#include <QString>
#include <qqmlintegration.h>

class LogInterface;
class SettingsManager;
class MachineRuntime;

class AlarmManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("AlarmManager is created in C++ and injected into the root QML object.")

public:
    explicit AlarmManager(LogInterface &logInterface,
                          SettingsManager &settings,
                          MachineRuntime &runtime,
                          QObject *parent = nullptr);

    Q_PROPERTY(QString alarmText READ alarmText NOTIFY alarmChanged)
    Q_PROPERTY(bool hasWarning READ hasWarning NOTIFY alarmChanged)
    Q_PROPERTY(bool isFault READ isFault NOTIFY alarmChanged)

    QString alarmText() const;
    bool hasWarning() const;
    bool isFault() const;

signals:
    void alarmChanged();
    void warningEntered(const QString &reason);
    void faultEntered(const QString &reason);
    void alarmCleared();

private:
    enum class AlarmLevel { Normal, Warning, Fault };
    void evaluateAlarm();
    void enterFault(const QString &reason);

    void appendLog(const QString &level, const QString &message);

private:
    AlarmLevel m_alarmLevel{AlarmLevel::Normal};
    QString m_alarmText{"System normal"};

    QPointer<LogInterface> m_logInterface{nullptr};
    QPointer<SettingsManager> m_settings{nullptr};
    QPointer<MachineRuntime> m_runtime{nullptr};
};
