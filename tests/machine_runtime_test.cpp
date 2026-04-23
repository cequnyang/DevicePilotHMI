#include <QtTest>

#include <QFile>
#include <QSignalSpy>
#include <QStandardPaths>

#include "log/log_interface.h"
#include "log/log_model.h"
#include "runtime/machine_runtime.h"
#include "settings/settings_file_store.h"
#include "test_machine_backend.h"

class MachineRuntimeTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();
    void startFlowTransitionsIdleToStartingToRunning();
};

void MachineRuntimeTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    QCoreApplication::setOrganizationName("DevicePilotHMITests");
    QCoreApplication::setApplicationName("DevicePilotHMI-MachineRuntimeTests");
}

void MachineRuntimeTest::cleanup()
{
    QFile::remove(Settings::Store::configFilePath());
}

void MachineRuntimeTest::startFlowTransitionsIdleToStartingToRunning()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    FakeMachineBackend backend;
    MachineRuntime runtime(logInterface, backend);
    QSignalSpy stateSpy(&runtime, &MachineRuntime::stateChanged);

    QCOMPARE(runtime.state(), MachineRuntime::State::Idle);
    QCOMPARE(runtime.status(), QString("Idle"));
    QVERIFY(runtime.canStart());
    QVERIFY(!runtime.canStop());
    QVERIFY(!runtime.canResetFault());
    QVERIFY(runtime.startDisabledReason().isEmpty());

    runtime.start();

    QCOMPARE(runtime.state(), MachineRuntime::State::Starting);
    QCOMPARE(runtime.status(), QString("Starting"));
    QVERIFY(!runtime.canStart());
    QVERIFY(runtime.canStop());
    QCOMPARE(runtime.startDisabledReason(), QString("Start request already in progress."));
    QCOMPARE(stateSpy.count(), 1);

    backend.publishState(MachineState::Running);

    QCOMPARE(runtime.state(), MachineRuntime::State::Running);
    QCOMPARE(runtime.status(), QString("Running"));
    QVERIFY(!runtime.canStart());
    QVERIFY(runtime.canStop());
    QCOMPARE(runtime.startDisabledReason(), QString("Machine is already running."));
    QCOMPARE(stateSpy.count(), 2);

    const QVariantList markers = runtime.historyMarkers();
    QCOMPARE(markers.size(), 2);
    QCOMPARE(markers.at(0).toMap().value("kind").toString(), QString("start"));
    QCOMPARE(markers.at(1).toMap().value("kind").toString(), QString("running"));
}

QTEST_APPLESS_MAIN(MachineRuntimeTest)
#include "machine_runtime_test.moc"
