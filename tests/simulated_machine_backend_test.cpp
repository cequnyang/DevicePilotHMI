#include <QtTest>

#include <QFile>
#include <QSignalSpy>
#include <QStandardPaths>

#include "backend/simulated_machine_backend.h"
#include "log/log_interface.h"
#include "log/log_model.h"
#include "runtime/machine_runtime.h"
#include "settings/settings_apply_service.h"
#include "settings/settings_file_store.h"
#include "settings/settings_manager.h"

class SimulatedMachineBackendTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();
    void appliedIdleUpdateIntervalChangesRunningTelemetryCadence();
};

void SimulatedMachineBackendTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    QCoreApplication::setOrganizationName("DevicePilotHMITests");
    QCoreApplication::setApplicationName("DevicePilotHMI-SimulatedMachineBackendTests");
}

void SimulatedMachineBackendTest::cleanup()
{
    QFile::remove(Settings::Store::configFilePath());
}

void SimulatedMachineBackendTest::appliedIdleUpdateIntervalChangesRunningTelemetryCadence()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    SimulatedMachineBackend backend(logInterface, settingsManager);
    MachineRuntime runtime(logInterface, backend);
    SettingsApplyService service(logInterface, settingsManager, runtime);

    Settings::Snapshot fastCadence = settingsManager.snapshot();
    fastCadence.updateIntervalMs = 100;
    QVERIFY(service.applySettings(fastCadence));
    QCOMPARE(settingsManager.updateIntervalMs(), 100);

    QSignalSpy telemetrySpy(&backend, &MachineBackend::telemetryReceived);

    runtime.start();
    QCOMPARE(runtime.state(), MachineRuntime::State::Starting);
    // Complete the five-second startup transition without making this cadence test slow.
    QVERIFY(QMetaObject::invokeMethod(&backend, "onTransitionTimeout", Qt::DirectConnection));
    QCOMPARE(runtime.state(), MachineRuntime::State::Running);

    telemetrySpy.clear();
    QTRY_VERIFY_WITH_TIMEOUT(telemetrySpy.count() >= 2, 350);
    QVERIFY(runtime.temperature() > RuntimeInit::kTemperature);
    QVERIFY(runtime.speed() > 700);
}

QTEST_GUILESS_MAIN(SimulatedMachineBackendTest)
#include "simulated_machine_backend_test.moc"
