#pragma once

#include <QDateTime>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QTimer>
#include <QVariant>
#include <qqmlintegration.h>

class MachineBackend;
class LogEvent;
class LogInterface;

#include "runtime/machine_types.h"

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
    Q_PROPERTY(QString startDisabledReason READ startDisabledReason NOTIFY stateChanged)
    Q_PROPERTY(QString stopDisabledReason READ stopDisabledReason NOTIFY stateChanged)
    Q_PROPERTY(QString resetDisabledReason READ resetDisabledReason NOTIFY stateChanged)

    Q_PROPERTY(QVariantList temperatureHistory READ temperatureHistory NOTIFY historyChanged)
    Q_PROPERTY(QVariantList pressureHistory READ pressureHistory NOTIFY historyChanged)
    Q_PROPERTY(QVariantList speedHistory READ speedHistory NOTIFY historyChanged)
    Q_PROPERTY(QVariantList historyMarkers READ historyMarkers NOTIFY historyChanged)
    Q_PROPERTY(int historyStartSampleIndex READ historyStartSampleIndex NOTIFY historyChanged)

    Q_PROPERTY(
        QString currentStateDurationText READ currentStateDurationText NOTIFY stateContextChanged)
    Q_PROPERTY(qint64 currentStateDurationSeconds READ currentStateDurationSeconds NOTIFY
                   stateContextChanged)
    Q_PROPERTY(QDateTime lastTransitionTime READ lastTransitionTime NOTIFY stateContextChanged)
    Q_PROPERTY(QString lastTransitionTimeText READ lastTransitionTimeText NOTIFY stateContextChanged)

public:
    using State = MachineState;

    explicit MachineRuntime(LogInterface &logInterface,
                            MachineBackend &backend,
                            QObject *parent = nullptr);

    QString status() const;
    double temperature() const;
    double pressure() const;
    int speed() const;

    bool canStart() const;
    bool canStop() const;
    bool canResetFault() const;
    QString startDisabledReason() const;
    QString stopDisabledReason() const;
    QString resetDisabledReason() const;

    QVariantList temperatureHistory() const;
    QVariantList pressureHistory() const;
    QVariantList speedHistory() const;
    QVariantList historyMarkers() const;
    int historyStartSampleIndex() const;

    QString currentStateDurationText() const;
    qint64 currentStateDurationSeconds() const;
    QDateTime lastTransitionTime() const;
    QString lastTransitionTimeText() const;

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
    void historyChanged();
    void stateContextChanged();

private slots:
    void onTelemetryReceived(TelemetryFrame frame);
    void onStateReported(MachineState state);

public:
    State state() const;
    void enterFault();
    void recordHistoryMarker(const QString &kind, const QString &label, const QString &color);

private:
    QString stateToString(State state) const;
    void appendLog(const LogEvent &event);
    void trimHistoryMarkers();
    void markStateTransition(const QDateTime &timestamp);

private:
    State m_state{State::Idle};
    bool m_faultResetPending{false};

    QDateTime m_stateEnteredAt{QDateTime::currentDateTime()};
    QDateTime m_lastTransitionTime;
    QTimer m_stateContextTimer;

    double m_temperature{RuntimeInit::kTemperature};
    double m_pressure{RuntimeInit::kPressure};
    int m_speed{RuntimeInit::kSpeed};

    QPointer<MachineBackend> m_backend{nullptr};
    QPointer<LogInterface> m_logInterface{nullptr};

    QVariantList m_temperatureHistory;
    QVariantList m_pressureHistory;
    QVariantList m_speedHistory;
    QVariantList m_historyMarkers;
    int m_nextSampleIndex{0};
    int m_historyStartSampleIndex{0};
    int m_lastSampleIndex{-1};
    static constexpr int kHistoryCapacity = 60;
};
