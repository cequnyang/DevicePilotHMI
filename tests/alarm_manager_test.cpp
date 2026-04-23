#include <QtTest>

#include <QFile>
#include <QSignalSpy>
#include <QStandardPaths>

#include "alarm/alarm_manager.h"
#include "log/log_interface.h"
#include "log/log_model.h"
#include "runtime/machine_runtime.h"
#include "settings/settings_apply_service.h"
#include "settings/settings_file_store.h"
#include "settings/settings_manager.h"
#include "test_machine_backend.h"

class AlarmManagerTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();
    void initialStateIsNormal();
    void idleStartingRunningWarningFaultResetRecoveryMainLink();
    void faultStateRemainsLatchedUntilReset();
    void resetTransitionsThroughRequestedAndRecoveryNotice();
    void warningTemperatureThresholdCrossingRaisesWarning();
    void warningTemperatureClearReturnsToNormalCopy();
    void thresholdApplyOnSecondMetricRaisesNewWarning();
    void thresholdApplyWhileRunningClearsWarningImmediately();
    void thresholdApplyWhileRunningRaisesFaultImmediately();
    void faultTemperatureThresholdCrossingEntersFault();
    void warningPressureThresholdCrossingRaisesWarning();
    void faultPressureThresholdCrossingEntersFault();
};

void AlarmManagerTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    QCoreApplication::setOrganizationName("DevicePilotHMITests");
    QCoreApplication::setApplicationName("DevicePilotHMI-AlarmManagerTests");
}

void AlarmManagerTest::cleanup()
{
    QFile::remove(Settings::Store::configFilePath());
}

void AlarmManagerTest::initialStateIsNormal()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    FakeMachineBackend backend;
    MachineRuntime runtime(logInterface, backend);
    AlarmManager alarm(logInterface, settingsManager, runtime);

    QVERIFY(!alarm.hasWarning());
    QVERIFY(!alarm.isFault());
    QVERIFY(!alarm.recoveryActive());
    QCOMPARE(alarm.alarmText(), QString("System normal"));
    QCOMPARE(alarm.stateLabel(), QString("Normal"));
    QCOMPARE(alarm.lifecycleState(), QString("Normal"));
}

void AlarmManagerTest::idleStartingRunningWarningFaultResetRecoveryMainLink()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    FakeMachineBackend backend;
    backend.setCompleteResetImmediately(false);
    MachineRuntime runtime(logInterface, backend);
    AlarmManager alarm(logInterface, settingsManager, runtime, nullptr, 25);

    Settings::Snapshot candidate = settingsManager.snapshot();
    candidate.warningTemperature = RuntimeInit::kTemperature + 1;
    candidate.faultTemperature = RuntimeInit::kTemperature + 3;
    QVERIFY(settingsManager.applySnapshot(candidate).ok);

    runtime.start();
    QCOMPARE(runtime.state(), MachineRuntime::State::Starting);
    QVERIFY(!alarm.hasWarning());
    QVERIFY(!alarm.isFault());

    backend.publishState(MachineState::Running);
    QCOMPARE(runtime.state(), MachineRuntime::State::Running);
    QCOMPARE(alarm.lifecycleState(), QString("Normal"));

    backend.publishTelemetry(RuntimeInit::kTemperature + 1.4, RuntimeInit::kPressure, 900);
    QVERIFY(alarm.hasWarning());
    QVERIFY(!alarm.isFault());
    QCOMPARE(alarm.lifecycleState(), QString("WarningActive"));
    QCOMPARE(alarm.activeMetric(), QString("temperature"));
    QCOMPARE(alarm.headline(), QString("Temperature warning"));

    backend.publishTelemetry(RuntimeInit::kTemperature + 3.5, RuntimeInit::kPressure, 1000);
    QVERIFY(alarm.isFault());
    QCOMPARE(runtime.state(), MachineRuntime::State::Fault);
    QCOMPARE(alarm.lifecycleState(), QString("FaultLatched"));
    QCOMPARE(alarm.headline(), QString("Over-temperature fault"));
    QVERIFY(alarm.detail().contains("while Running"));

    runtime.resetFault();
    QVERIFY(runtime.faultResetPending());
    QCOMPARE(alarm.lifecycleState(), QString("ResetRequested"));
    QCOMPARE(alarm.stateLabel(), QString("Resetting"));

    backend.completePendingReset();
    QCOMPARE(runtime.state(), MachineRuntime::State::Idle);
    QVERIFY(!runtime.faultResetPending());
    QVERIFY(alarm.recoveryActive());
    QVERIFY(!alarm.isFault());
    QCOMPARE(alarm.lifecycleState(), QString("RecoveryNotice"));
    QCOMPARE(alarm.headline(), QString("Fault cleared"));
}

void AlarmManagerTest::faultStateRemainsLatchedUntilReset()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    FakeMachineBackend backend;
    MachineRuntime runtime(logInterface, backend);
    AlarmManager alarm(logInterface, settingsManager, runtime);

    Settings::Snapshot candidate = settingsManager.snapshot();
    candidate.warningTemperature = RuntimeInit::kTemperature + 1;
    candidate.faultTemperature = RuntimeInit::kTemperature + 2;
    const auto apply = settingsManager.applySnapshot(candidate);
    QVERIFY(apply.ok);

    backend.publishState(MachineState::Running);
    backend.publishTelemetry(RuntimeInit::kTemperature + 2.6, RuntimeInit::kPressure, 800);

    QVERIFY(alarm.isFault());
    QCOMPARE(runtime.state(), MachineRuntime::State::Fault);

    backend.publishState(MachineState::Starting);
    QCOMPARE(runtime.state(), MachineRuntime::State::Fault);

    backend.publishState(MachineState::Running);
    QCOMPARE(runtime.state(), MachineRuntime::State::Fault);

    backend.publishState(MachineState::Stopping);
    QCOMPARE(runtime.state(), MachineRuntime::State::Fault);

    backend.publishState(MachineState::Idle);
    QCOMPARE(runtime.state(), MachineRuntime::State::Fault);
}

void AlarmManagerTest::resetTransitionsThroughRequestedAndRecoveryNotice()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    FakeMachineBackend backend;
    backend.setCompleteResetImmediately(false);
    MachineRuntime runtime(logInterface, backend);
    AlarmManager alarm(logInterface, settingsManager, runtime, nullptr, 25);

    Settings::Snapshot candidate = settingsManager.snapshot();
    candidate.warningTemperature = RuntimeInit::kTemperature + 1;
    candidate.faultTemperature = RuntimeInit::kTemperature + 2;
    const auto apply = settingsManager.applySnapshot(candidate);
    QVERIFY(apply.ok);

    backend.publishState(MachineState::Running);
    backend.publishTelemetry(RuntimeInit::kTemperature + 2.6, RuntimeInit::kPressure, 800);

    QVERIFY(alarm.isFault());
    QCOMPARE(alarm.lifecycleState(), QString("FaultLatched"));

    runtime.resetFault();

    QVERIFY(runtime.faultResetPending());
    QVERIFY(!runtime.canResetFault());
    QCOMPARE(alarm.lifecycleState(), QString("ResetRequested"));
    QCOMPARE(alarm.stateLabel(), QString("Resetting"));
    QVERIFY(alarm.isFault());

    backend.completePendingReset();

    QCOMPARE(runtime.state(), MachineRuntime::State::Idle);
    QVERIFY(!runtime.faultResetPending());
    QVERIFY(alarm.recoveryActive());
    QVERIFY(!alarm.isFault());
    QCOMPARE(alarm.lifecycleState(), QString("RecoveryNotice"));
    QCOMPARE(alarm.stateLabel(), QString("Recovered"));
    QCOMPARE(alarm.headline(), QString("Fault cleared"));
}

void AlarmManagerTest::warningTemperatureThresholdCrossingRaisesWarning()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    FakeMachineBackend backend;
    MachineRuntime runtime(logInterface, backend);
    AlarmManager alarm(logInterface, settingsManager, runtime);

    Settings::Snapshot candidate = settingsManager.snapshot();
    candidate.warningTemperature = RuntimeInit::kTemperature + 1;
    candidate.faultTemperature = RuntimeInit::kTemperature + 40;
    const auto apply = settingsManager.applySnapshot(candidate);
    QVERIFY(apply.ok);

    backend.publishState(MachineState::Running);
    backend.publishTelemetry(RuntimeInit::kTemperature + 1.6, RuntimeInit::kPressure, 800);

    QVERIFY(alarm.hasWarning());
    QVERIFY(!alarm.isFault());
    QCOMPARE(alarm.lifecycleState(), QString("WarningActive"));
    QCOMPARE(alarm.headline(), QString("Temperature warning"));
    QVERIFY(alarm.detail().contains("warning limit"));
    QVERIFY(alarm.detail().contains("trend rising"));
}

void AlarmManagerTest::warningTemperatureClearReturnsToNormalCopy()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    FakeMachineBackend backend;
    MachineRuntime runtime(logInterface, backend);
    AlarmManager alarm(logInterface, settingsManager, runtime);

    Settings::Snapshot candidate = settingsManager.snapshot();
    candidate.warningTemperature = RuntimeInit::kTemperature + 1;
    candidate.faultTemperature = RuntimeInit::kTemperature + 40;
    const auto apply = settingsManager.applySnapshot(candidate);
    QVERIFY(apply.ok);

    backend.publishState(MachineState::Running);
    backend.publishTelemetry(RuntimeInit::kTemperature + 1.6, RuntimeInit::kPressure, 800);
    QVERIFY(alarm.hasWarning());

    backend.publishTelemetry(RuntimeInit::kTemperature, RuntimeInit::kPressure, 800);

    QVERIFY(!alarm.hasWarning());
    QVERIFY(!alarm.recoveryActive());
    QCOMPARE(alarm.lifecycleState(), QString("Normal"));
    QCOMPARE(alarm.headline(), QString("System normal"));
}

void AlarmManagerTest::thresholdApplyOnSecondMetricRaisesNewWarning()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    FakeMachineBackend backend;
    MachineRuntime runtime(logInterface, backend);
    AlarmManager alarm(logInterface, settingsManager, runtime);
    QSignalSpy warningSpy(&alarm, &AlarmManager::warningEntered);

    Settings::Snapshot candidate = settingsManager.snapshot();
    candidate.warningTemperature = RuntimeInit::kTemperature + 1;
    candidate.faultTemperature = RuntimeInit::kTemperature + 40;
    candidate.warningPressure = RuntimeInit::kPressure + 20;
    candidate.faultPressure = RuntimeInit::kPressure + 40;
    QVERIFY(settingsManager.applySnapshot(candidate).ok);

    backend.publishState(MachineState::Running);
    backend.publishTelemetry(RuntimeInit::kTemperature + 1.6, RuntimeInit::kPressure + 10.0, 800);

    QCOMPARE(warningSpy.count(), 1);
    QVERIFY(alarm.hasWarning());
    QCOMPARE(alarm.activeMetric(), QString("temperature"));
    QCOMPARE(alarm.headline(), QString("Temperature warning"));

    candidate.warningPressure = RuntimeInit::kPressure + 5;
    QVERIFY(settingsManager.applySnapshot(candidate).ok);

    QCOMPARE(warningSpy.count(), 2);
    QVERIFY(alarm.hasWarning());
    QVERIFY(!alarm.isFault());
    QCOMPARE(alarm.activeMetric(), QString("pressure"));
    QCOMPARE(alarm.headline(), QString("Pressure warning"));
    QVERIFY(alarm.detail().contains("Pressure"));

    backend.publishTelemetry(RuntimeInit::kTemperature + 1.8, RuntimeInit::kPressure + 10.2, 800);

    QCOMPARE(warningSpy.count(), 2);
    QCOMPARE(alarm.activeMetric(), QString("pressure"));
    QCOMPARE(alarm.headline(), QString("Pressure warning"));
}

void AlarmManagerTest::thresholdApplyWhileRunningClearsWarningImmediately()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    FakeMachineBackend backend;
    MachineRuntime runtime(logInterface, backend);
    SettingsApplyService service(logInterface, settingsManager, runtime);
    AlarmManager alarm(logInterface, settingsManager, runtime);
    QSignalSpy warningClearedSpy(&alarm, &AlarmManager::warningCleared);

    Settings::Snapshot candidate = settingsManager.snapshot();
    candidate.warningTemperature = RuntimeInit::kTemperature + 1;
    candidate.faultTemperature = RuntimeInit::kTemperature + 40;
    QVERIFY(service.applySettings(candidate));

    runtime.start();
    backend.publishState(MachineState::Running);
    backend.publishTelemetry(RuntimeInit::kTemperature + 1.6, RuntimeInit::kPressure, 800);
    QVERIFY(alarm.hasWarning());
    QCOMPARE(alarm.activeMetric(), QString("temperature"));

    candidate.warningTemperature = RuntimeInit::kTemperature + 10;
    candidate.faultTemperature = RuntimeInit::kTemperature + 40;
    QVERIFY(service.applySettings(candidate));

    QCOMPARE(warningClearedSpy.count(), 1);
    QVERIFY(!alarm.hasWarning());
    QVERIFY(!alarm.isFault());
    QCOMPARE(alarm.lifecycleState(), QString("Normal"));
    QCOMPARE(alarm.headline(), QString("System normal"));
}

void AlarmManagerTest::thresholdApplyWhileRunningRaisesFaultImmediately()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    FakeMachineBackend backend;
    MachineRuntime runtime(logInterface, backend);
    SettingsApplyService service(logInterface, settingsManager, runtime);
    AlarmManager alarm(logInterface, settingsManager, runtime);
    QSignalSpy faultSpy(&alarm, &AlarmManager::faultEntered);

    Settings::Snapshot candidate = settingsManager.snapshot();
    candidate.warningTemperature = RuntimeInit::kTemperature + 10;
    candidate.faultTemperature = RuntimeInit::kTemperature + 40;
    QVERIFY(service.applySettings(candidate));

    runtime.start();
    backend.publishState(MachineState::Running);
    backend.publishTelemetry(RuntimeInit::kTemperature + 8.0, RuntimeInit::kPressure, 800);
    QVERIFY(!alarm.hasWarning());
    QVERIFY(!alarm.isFault());

    candidate.warningTemperature = RuntimeInit::kTemperature + 1;
    candidate.faultTemperature = RuntimeInit::kTemperature + 5;
    QVERIFY(service.applySettings(candidate));

    QCOMPARE(faultSpy.count(), 1);
    QVERIFY(alarm.isFault());
    QCOMPARE(runtime.state(), MachineRuntime::State::Fault);
    QCOMPARE(alarm.lifecycleState(), QString("FaultLatched"));
    QCOMPARE(alarm.headline(), QString("Over-temperature fault"));
    QCOMPARE(alarm.activeMetric(), QString("temperature"));
}

void AlarmManagerTest::faultTemperatureThresholdCrossingEntersFault()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    FakeMachineBackend backend;
    MachineRuntime runtime(logInterface, backend);
    AlarmManager alarm(logInterface, settingsManager, runtime);

    Settings::Snapshot candidate = settingsManager.snapshot();
    candidate.warningTemperature = RuntimeInit::kTemperature + 1;
    candidate.faultTemperature = RuntimeInit::kTemperature + 2;
    const auto apply = settingsManager.applySnapshot(candidate);
    QVERIFY(apply.ok);

    backend.publishState(MachineState::Running);
    backend.publishTelemetry(RuntimeInit::kTemperature + 2.6, RuntimeInit::kPressure, 800);

    QVERIFY(alarm.isFault());
    QCOMPARE(runtime.state(), MachineRuntime::State::Fault);
    QCOMPARE(alarm.lifecycleState(), QString("FaultLatched"));
    QCOMPARE(alarm.alarmText(), QString("Over-temperature fault"));
    QVERIFY(alarm.detail().contains("exceeded fault limit"));
    QVERIFY(alarm.operatorHint().contains("Reset Fault"));
}

void AlarmManagerTest::warningPressureThresholdCrossingRaisesWarning()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    FakeMachineBackend backend;
    MachineRuntime runtime(logInterface, backend);
    AlarmManager alarm(logInterface, settingsManager, runtime);

    Settings::Snapshot candidate = settingsManager.snapshot();
    candidate.warningPressure = RuntimeInit::kPressure + 1;
    candidate.faultPressure = RuntimeInit::kPressure + 40;
    const auto apply = settingsManager.applySnapshot(candidate);
    QVERIFY(apply.ok);

    backend.publishState(MachineState::Running);
    backend.publishTelemetry(RuntimeInit::kTemperature, RuntimeInit::kPressure + 2.4, 800);

    QVERIFY(alarm.hasWarning());
    QVERIFY(!alarm.isFault());
}

void AlarmManagerTest::faultPressureThresholdCrossingEntersFault()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    FakeMachineBackend backend;
    MachineRuntime runtime(logInterface, backend);
    AlarmManager alarm(logInterface, settingsManager, runtime);

    Settings::Snapshot candidate = settingsManager.snapshot();
    candidate.warningPressure = RuntimeInit::kPressure + 1;
    candidate.faultPressure = RuntimeInit::kPressure + 2;
    const auto apply = settingsManager.applySnapshot(candidate);
    QVERIFY(apply.ok);

    backend.publishState(MachineState::Running);
    backend.publishTelemetry(RuntimeInit::kTemperature, RuntimeInit::kPressure + 2.4, 800);

    QVERIFY(alarm.isFault());
    QCOMPARE(runtime.state(), MachineRuntime::State::Fault);
    QCOMPARE(alarm.lifecycleState(), QString("FaultLatched"));
    QCOMPARE(alarm.alarmText(), QString("Over-pressure fault"));
    QVERIFY(alarm.detail().contains("exceeded fault limit"));
}

QTEST_APPLESS_MAIN(AlarmManagerTest)
#include "alarm_manager_test.moc"
