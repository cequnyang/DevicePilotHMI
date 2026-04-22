#include <QtTest>

#include <QFile>
#include <QStandardPaths>

#include "log/log_interface.h"
#include "log/log_model.h"
#include "settings/settings_draft.h"
#include "settings/settings_file_store.h"
#include "settings/settings_manager.h"

class SettingsDraftTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();
    void initialDraftIsValidAndClean();
    void loadSnapshotUpdatesDraftWithoutResettingOriginalSnapshot();
    void warningTemperatureAtFaultBoundaryIsClampedWithMessage();
    void faultTemperatureAtWarningBoundaryIsClampedWithMessage();
    void warningPressureAtFaultBoundaryIsClampedWithMessage();
    void faultPressureAtWarningBoundaryIsClampedWithMessage();
    void validEditClearsPreviousValidationMessage();
    void loadFromClearsValidationMessageAndDirtyState();
    void resetDraftToDefaultsClearsValidationMessage();
};

void SettingsDraftTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    QCoreApplication::setOrganizationName("DevicePilotHMITests");
    QCoreApplication::setApplicationName("DevicePilotHMI-SettingsDraftTests");
}

void SettingsDraftTest::cleanup()
{
    QFile::remove(Settings::Store::configFilePath());
}

void SettingsDraftTest::initialDraftIsValidAndClean()
{
    SettingsDraft draft;

    QVERIFY(draft.valid());
    QVERIFY(!draft.dirty());
    QVERIFY(!draft.canApply());
    QVERIFY(draft.validationMessage().isEmpty());
}

void SettingsDraftTest::loadSnapshotUpdatesDraftWithoutResettingOriginalSnapshot()
{
    SettingsDraft draft;

    draft.setWarningTemperature(draft.faultTemperature());
    QVERIFY(!draft.validationMessage().isEmpty());

    const Settings::Snapshot replacement{78, 98, 118, 138, 1200};
    draft.loadSnapshot(replacement);

    QCOMPARE(draft.snapshot(), replacement);
    QVERIFY(draft.dirty());
    QVERIFY(draft.canApply());
    QVERIFY(draft.validationMessage().isEmpty());
}

void SettingsDraftTest::warningTemperatureAtFaultBoundaryIsClampedWithMessage()
{
    SettingsDraft draft;

    draft.setWarningTemperature(draft.faultTemperature());

    QCOMPARE(draft.warningTemperature(), draft.faultTemperature() - 1);
    QCOMPARE(draft.validationMessage(),
             QString("Warning temperature must be lower than fault temperature."));
    QVERIFY(draft.valid());
    QVERIFY(draft.dirty());
    QVERIFY(draft.canApply());
}

void SettingsDraftTest::faultTemperatureAtWarningBoundaryIsClampedWithMessage()
{
    SettingsDraft draft;

    draft.setFaultTemperature(draft.warningTemperature());

    QCOMPARE(draft.faultTemperature(), draft.warningTemperature() + 1);
    QCOMPARE(draft.validationMessage(),
             QString("Warning temperature must be lower than fault temperature."));
    QVERIFY(draft.valid());
    QVERIFY(draft.dirty());
    QVERIFY(draft.canApply());
}

void SettingsDraftTest::warningPressureAtFaultBoundaryIsClampedWithMessage()
{
    SettingsDraft draft;

    draft.setWarningPressure(draft.faultPressure());

    QCOMPARE(draft.warningPressure(), draft.faultPressure() - 1);
    QCOMPARE(draft.validationMessage(),
             QString("Warning pressure must be lower than fault pressure."));
    QVERIFY(draft.valid());
    QVERIFY(draft.dirty());
    QVERIFY(draft.canApply());
}

void SettingsDraftTest::faultPressureAtWarningBoundaryIsClampedWithMessage()
{
    SettingsDraft draft;

    draft.setFaultPressure(draft.warningPressure());

    QCOMPARE(draft.faultPressure(), draft.warningPressure() + 1);
    QCOMPARE(draft.validationMessage(),
             QString("Warning pressure must be lower than fault pressure."));
    QVERIFY(draft.valid());
    QVERIFY(draft.dirty());
    QVERIFY(draft.canApply());
}

void SettingsDraftTest::validEditClearsPreviousValidationMessage()
{
    SettingsDraft draft;

    draft.setWarningTemperature(draft.faultTemperature());
    QVERIFY(!draft.validationMessage().isEmpty());

    draft.setWarningTemperature(80);

    QVERIFY(draft.validationMessage().isEmpty());
    QVERIFY(draft.valid());
    QVERIFY(draft.canApply());
}

void SettingsDraftTest::loadFromClearsValidationMessageAndDirtyState()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    SettingsDraft draft;

    Settings::Snapshot candidate = settingsManager.snapshot();
    candidate.warningTemperature += 10;
    QVERIFY(settingsManager.applySnapshot(candidate).ok);

    draft.setWarningTemperature(draft.faultTemperature());
    QVERIFY(!draft.validationMessage().isEmpty());
    QVERIFY(draft.dirty());

    draft.loadFrom(&settingsManager);

    QCOMPARE(draft.warningTemperature(), candidate.warningTemperature);
    QCOMPARE(draft.faultTemperature(), candidate.faultTemperature);
    QCOMPARE(draft.warningPressure(), candidate.warningPressure);
    QCOMPARE(draft.faultPressure(), candidate.faultPressure);
    QCOMPARE(draft.updateIntervalMs(), candidate.updateIntervalMs);
    QVERIFY(draft.validationMessage().isEmpty());
    QVERIFY(!draft.dirty());
    QVERIFY(!draft.canApply());
}

void SettingsDraftTest::resetDraftToDefaultsClearsValidationMessage()
{
    SettingsDraft draft;

    draft.setWarningTemperature(draft.faultTemperature());
    QVERIFY(!draft.validationMessage().isEmpty());

    draft.resetDraftToDefaults();

    QCOMPARE(draft.snapshot(), Settings::defaults());
    QVERIFY(draft.validationMessage().isEmpty());
    QVERIFY(!draft.dirty());
    QVERIFY(!draft.canApply());
}

QTEST_APPLESS_MAIN(SettingsDraftTest)
#include "settings_draft_test.moc"
