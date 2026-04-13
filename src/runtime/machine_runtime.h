#pragma once

#include <QObject>
#include <QPointer>
#include <QString>
#include <QTimer>
#include <qqmlintegration.h>

class LogInterface;
class SettingsManager;

namespace RuntimeInit {
static constexpr int kTemperature = 25;
static constexpr int kPressure = 80;
static constexpr int kSpeed = 0;
}; // namespace RuntimeInit

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
    enum class PendingTransition { None, FinishStart, FinishStop };

    void setState(State newState);
    QString stateToString(State state) const;
    void resetMeasurementsToIdle();
    void appendLog(const QString &level, const QString &message);

private:
    State m_state{State::Idle};
    PendingTransition m_pendingTransition{PendingTransition::None};

    double m_temperature{RuntimeInit::kTemperature};
    double m_pressure{RuntimeInit::kPressure};
    int m_speed{RuntimeInit::kSpeed};

    QTimer m_updateTimer;
    QTimer m_transitionTimer;

    QPointer<LogInterface> m_logInterface{nullptr};
    QPointer<SettingsManager> m_settings{nullptr};
};
