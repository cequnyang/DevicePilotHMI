#pragma once

#include <QDateTime>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QTimer>
#include <qqmlintegration.h>

class LogInterface;
struct LogEvent;
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
                          QObject *parent = nullptr,
                          int recoveryNoticeDurationMs = 4000);

    Q_PROPERTY(QString alarmText READ alarmText NOTIFY alarmChanged)
    Q_PROPERTY(QString headline READ headline NOTIFY alarmChanged)
    Q_PROPERTY(QString detail READ detail NOTIFY alarmChanged)
    Q_PROPERTY(QString operatorHint READ operatorHint NOTIFY alarmChanged)
    Q_PROPERTY(QString stateLabel READ stateLabel NOTIFY alarmChanged)
    Q_PROPERTY(QString lifecycleState READ lifecycleState NOTIFY alarmChanged)
    Q_PROPERTY(bool hasWarning READ hasWarning NOTIFY alarmChanged)
    Q_PROPERTY(bool isFault READ isFault NOTIFY alarmChanged)
    Q_PROPERTY(bool recoveryActive READ recoveryActive NOTIFY alarmChanged)
    Q_PROPERTY(QString activeMetric READ activeMetric NOTIFY alarmChanged)

    QString alarmText() const;
    QString headline() const;
    QString detail() const;
    QString operatorHint() const;
    QString stateLabel() const;
    QString lifecycleState() const;
    bool hasWarning() const;
    bool isFault() const;
    bool recoveryActive() const;
    QString activeMetric() const;

signals:
    void alarmChanged();
    void warningEntered(const QString &message);
    void warningCleared();
    void faultEntered(const QString &message);
    void recoveryEntered();

private:
    enum class AlarmLifecycle { Normal, WarningActive, FaultLatched, ResetRequested, RecoveryNotice };

    struct MetricContext
    {
        QString key;
        double currentValue{0.0};
        double warningThreshold{0.0};
        double faultThreshold{0.0};
        QString trend;
    };

    void evaluateAlarm();
    void enterNormal();
    void enterWarning(const MetricContext &context, bool emitEntrySignal);
    void enterFault(const MetricContext &context);
    void enterResetRequested();
    void enterRecoveryNotice();
    void setPresentation(AlarmLifecycle lifecycle,
                         const QString &headline,
                         const QString &detail,
                         const QString &operatorHint,
                         const QString &stateLabel,
                         const QString &activeMetric);
    MetricContext makeMetricContext(const QString &metricKey,
                                    double currentValue,
                                    double warningThreshold,
                                    double faultThreshold) const;
    QString selectWarningMetric(bool temperatureWarning,
                                bool pressureWarning,
                                bool temperatureWarningEdge,
                                bool pressureWarningEdge) const;
    QString selectFaultMetric(bool temperatureFault, bool pressureFault) const;
    QString metricLabel(const QString &metricKey) const;
    QString formatMetricValue(const QString &metricKey, double value) const;
    QString formatWarningDetail(const MetricContext &context,
                               const QDateTime &warningStartedAt) const;
    QString formatFaultDetail(const MetricContext &context,
                              const QString &triggerState,
                              bool warningWasActive,
                              qint64 warningDurationSeconds) const;
    QString trendForMetric(const QString &metricKey, double currentValue) const;
    QDateTime warningStartedAtForMetric(const QString &metricKey) const;
    void clearWarningTracking();
    void updateObservedValues(double temperature, double pressure);

    void appendLog(const LogEvent &event);

private:
    AlarmLifecycle m_lifecycle{AlarmLifecycle::Normal};
    QString m_headline{"System normal"};
    QString m_detail{"Temperature and pressure are within configured limits."};
    QString m_operatorHint;
    QString m_stateLabel{"Normal"};
    QString m_activeMetric;
    QDateTime m_temperatureWarningStartedAt;
    QDateTime m_pressureWarningStartedAt;
    bool m_temperatureWarningActive{false};
    bool m_pressureWarningActive{false};
    double m_lastObservedTemperature{0.0};
    double m_lastObservedPressure{0.0};
    bool m_hasObservedValues{false};
    QString m_lastFaultMetric;
    QTimer m_recoveryTimer;

    QPointer<LogInterface> m_logInterface{nullptr};
    QPointer<SettingsManager> m_settings{nullptr};
    QPointer<MachineRuntime> m_runtime{nullptr};
};
