#include <QtTest>

#include <QFile>
#include <QStandardPaths>

#include "log/log_interface.h"
#include "log/log_model.h"
#include "settings/settings_file_store.h"
#include "settings/settings_manager.h"

class SettingsManagerTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();
    void defaultStartupShowsAllLogColumns();
    void logColumnVisibilityPersistsAcrossManagerInstances();
    void applyingSnapshotPreservesLogColumnVisibility();
};

void SettingsManagerTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    QCoreApplication::setOrganizationName("DevicePilotHMITests");
    QCoreApplication::setApplicationName("DevicePilotHMI-SettingsManagerTests");
}

void SettingsManagerTest::cleanup()
{
    QFile::remove(Settings::Store::configFilePath());
}

void SettingsManagerTest::defaultStartupShowsAllLogColumns()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);

    QVERIFY(settingsManager.showTimestamp());
    QVERIFY(settingsManager.showSource());
    QVERIFY(settingsManager.showLevel());

    const auto loaded = Settings::Store::loadConfig();
    QCOMPARE(loaded.config.logViewPreferences, Settings::defaultLogViewPreferences());
}

void SettingsManagerTest::logColumnVisibilityPersistsAcrossManagerInstances()
{
    {
        LogModel logModel;
        LogInterface logInterface(logModel);
        SettingsManager settingsManager(logInterface);

        settingsManager.setShowTimestamp(false);
        settingsManager.setShowSource(false);
        settingsManager.setShowLevel(false);

        const auto loaded = Settings::Store::loadConfig();
        QVERIFY(!loaded.config.logViewPreferences.showTimestamp);
        QVERIFY(!loaded.config.logViewPreferences.showSource);
        QVERIFY(!loaded.config.logViewPreferences.showLevel);
        QVERIFY(!loaded.repaired);
    }

    LogModel restoredLogModel;
    LogInterface restoredLogInterface(restoredLogModel);
    SettingsManager restoredSettingsManager(restoredLogInterface);

    QVERIFY(!restoredSettingsManager.showTimestamp());
    QVERIFY(!restoredSettingsManager.showSource());
    QVERIFY(!restoredSettingsManager.showLevel());
}

void SettingsManagerTest::applyingSnapshotPreservesLogColumnVisibility()
{
    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);

    settingsManager.setShowTimestamp(false);
    settingsManager.setShowLevel(false);

    Settings::Snapshot candidate = settingsManager.snapshot();
    candidate.warningTemperature += 5;
    candidate.faultTemperature += 5;
    candidate.warningPressure += 2;
    candidate.faultPressure += 2;
    candidate.updateIntervalMs += 100;

    QVERIFY(settingsManager.applySnapshot(candidate));

    const auto loaded = Settings::Store::loadConfig();
    QCOMPARE(loaded.config.snapshot, candidate);
    QVERIFY(!loaded.config.logViewPreferences.showTimestamp);
    QVERIFY(loaded.config.logViewPreferences.showSource);
    QVERIFY(!loaded.config.logViewPreferences.showLevel);
    QVERIFY(!loaded.repaired);
}

QTEST_APPLESS_MAIN(SettingsManagerTest)
#include "settings_manager_test.moc"
