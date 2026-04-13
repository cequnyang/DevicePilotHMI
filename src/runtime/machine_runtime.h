#pragma once

#include <QObject>
#include <QPointer>
#include <QString>
#include <QTimer>
#include <qqmlintegration.h>

class LogInterface;
class SettingsManager;

class MachineRuntime : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("MachineRuntime is created in C++ and injected into the root QML object.")

    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(double temperature READ temperature NOTIFY temperatureChanged)
    Q_PROPERTY(double pressure READ pressure NOTIFY pressureChanged)
    Q_PROPERTY(int speed READ speed NOTIFY speedChanged)

    Q_PROPERTY(bool canStart READ canStart NOTIFY stateChanged)
    Q_PROPERTY(bool canStop READ canStop NOTIFY stateChanged)
    Q_PROPERTY(bool canResetFault READ canResetFault NOTIFY stateChanged)

public:
    explicit MachineRuntime(LogInterface &logInterface,
                            SettingsManager &settings,
                            QObject *parent = nullptr);

    QString status() const;
    double temperature() const;
    double pressure() const;
    int speed() const;

    bool canStart() const;
    bool canStop() const;
    bool canResetFault() const;

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void resetFault();

signals:
    void statusChanged();
    void temperatureChanged();
    void pressureChanged();
    void speedChanged();
    void stateChanged();
    void evaluateAlarm();
    void resetAlarmState();

private slots:
    void updateSimulation();
    void onTransitionTimeout();

public:
    enum class State { Idle, Starting, Running, Stopping, Fault };
    State state() const;
    void onEnterFault();

private:
    static constexpr int kInitTemperature = 25;
    static constexpr int kInitPressure = 80;
    static constexpr int kInitSpeed = 0;

    enum class PendingTransition { None, FinishStart, FinishStop };

    void setState(State newState);
    QString stateToString(State state) const;
    void resetMeasurementsToIdle();
    void appendLog(const QString &level, const QString &message);

private:
    State m_state{State::Idle};
    PendingTransition m_pendingTransition{PendingTransition::None};

    double m_temperature{25.0};
    double m_pressure{80.0};
    int m_speed{0};

    QTimer m_updateTimer;
    QTimer m_transitionTimer;

    QPointer<LogInterface> m_logInterface{nullptr};
    QPointer<SettingsManager> m_settings{nullptr};
};