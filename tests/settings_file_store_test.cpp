#include <QtTest>

#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

#include "settings/settings_file_store.h"

namespace {
void writeRawSettingsFile(const QByteArray &raw)
{
    QFile file(Settings::Store::configFilePath());
    QVERIFY2(file.open(QIODevice::WriteOnly | QIODevice::Truncate),
             qPrintable(file.errorString()));
    QCOMPARE(file.write(raw), raw.size());
    file.close();
}
} // namespace

class SettingsFileStoreTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();
    void persistAndLoadRoundTrip();
    void missingFileReturnsDefaultsAndMarksRepaired();
    void invalidJsonReturnsDefaultsAndMarksRepaired();
    void nonObjectRootReturnsDefaultsAndMarksRepaired();
    void invalidValuesReturnDefaultsAndMarksRepaired();
    void missingLogPageReturnsSnapshotAndDefaultPreferences();
    void invalidLogPageRepairsPreferencesWithoutDiscardingSnapshot();
};

void SettingsFileStoreTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    QCoreApplication::setOrganizationName("DevicePilotHMITests");
    QCoreApplication::setApplicationName("DevicePilotHMI-SettingsFileStoreTests");
}

void SettingsFileStoreTest::cleanup()
{
    QFile::remove(Settings::Store::configFilePath());
}

void SettingsFileStoreTest::persistAndLoadRoundTrip()
{
    Settings::PersistedConfig config = Settings::defaultsConfig();
    config.snapshot.warningTemperature = 90;
    config.snapshot.faultTemperature = 120;
    config.snapshot.warningPressure = 130;
    config.snapshot.faultPressure = 145;
    config.snapshot.updateIntervalMs = 1500;
    config.logViewPreferences.showTimestamp = false;
    config.logViewPreferences.showSource = false;

    const auto persist = Settings::Store::persistConfig(config);
    QVERIFY(persist.ok);
    QVERIFY(persist.reason.isEmpty());
    QVERIFY(QFileInfo::exists(Settings::Store::configFilePath()));

    const auto load = Settings::Store::loadConfig();
    QCOMPARE(load.config, config);
    QVERIFY(!load.repaired);
    QVERIFY(load.reason.isEmpty());
}

void SettingsFileStoreTest::missingFileReturnsDefaultsAndMarksRepaired()
{
    QFile::remove(Settings::Store::configFilePath());

    const auto load = Settings::Store::loadConfig();

    QCOMPARE(load.config, Settings::defaultsConfig());
    QVERIFY(load.repaired);
    QVERIFY(load.reason.contains("does not exist", Qt::CaseInsensitive));
}

void SettingsFileStoreTest::invalidJsonReturnsDefaultsAndMarksRepaired()
{
    writeRawSettingsFile("{ definitely-not-json");

    const auto load = Settings::Store::loadConfig();

    QCOMPARE(load.config, Settings::defaultsConfig());
    QVERIFY(load.repaired);
    QVERIFY(load.reason.contains("Invalid settings.json structure", Qt::CaseInsensitive));
    QVERIFY(load.reason.contains("Invalid JSON", Qt::CaseInsensitive));
}

void SettingsFileStoreTest::nonObjectRootReturnsDefaultsAndMarksRepaired()
{
    writeRawSettingsFile("[1, 2, 3]");

    const auto load = Settings::Store::loadConfig();

    QCOMPARE(load.config, Settings::defaultsConfig());
    QVERIFY(load.repaired);
    QVERIFY(load.reason.contains("Invalid settings.json structure", Qt::CaseInsensitive));
    QVERIFY(load.reason.contains("not a JSON object", Qt::CaseInsensitive));
}

void SettingsFileStoreTest::invalidValuesReturnDefaultsAndMarksRepaired()
{
    writeRawSettingsFile(R"({
        "schemaVersion": 1,
        "warningTemperature": 90,
        "faultTemperature": 80,
        "warningPressure": 115,
        "faultPressure": 135,
        "updateIntervalMs": 1000,
        "logPage": {
            "showTimestamp": false,
            "showSource": true,
            "showLevel": false
        }
    })");

    const auto load = Settings::Store::loadConfig();

    QCOMPARE(load.config, Settings::defaultsConfig());
    QVERIFY(load.repaired);
    QVERIFY(load.reason.contains("Invalid settings values", Qt::CaseInsensitive));
    QVERIFY(load.reason.contains("Warning temperature must be lower than fault temperature.",
                                 Qt::CaseInsensitive));
}

void SettingsFileStoreTest::missingLogPageReturnsSnapshotAndDefaultPreferences()
{
    writeRawSettingsFile(R"({
        "schemaVersion": 1,
        "warningTemperature": 88,
        "faultTemperature": 108,
        "warningPressure": 126,
        "faultPressure": 142,
        "updateIntervalMs": 1600
    })");

    const auto load = Settings::Store::loadConfig();

    QCOMPARE(load.config.snapshot.warningTemperature, 88);
    QCOMPARE(load.config.snapshot.faultTemperature, 108);
    QCOMPARE(load.config.snapshot.warningPressure, 126);
    QCOMPARE(load.config.snapshot.faultPressure, 142);
    QCOMPARE(load.config.snapshot.updateIntervalMs, 1600);
    QCOMPARE(load.config.logViewPreferences, Settings::defaultLogViewPreferences());
    QVERIFY(load.repaired);
    QVERIFY(load.reason.contains("logPage"));
}

void SettingsFileStoreTest::invalidLogPageRepairsPreferencesWithoutDiscardingSnapshot()
{
    writeRawSettingsFile(R"({
        "schemaVersion": 1,
        "warningTemperature": 86,
        "faultTemperature": 102,
        "warningPressure": 124,
        "faultPressure": 144,
        "updateIntervalMs": 1700,
        "logPage": {
            "showTimestamp": false,
            "showSource": 1
        }
    })");

    const auto load = Settings::Store::loadConfig();

    QCOMPARE(load.config.snapshot.warningTemperature, 86);
    QCOMPARE(load.config.snapshot.faultTemperature, 102);
    QCOMPARE(load.config.snapshot.warningPressure, 124);
    QCOMPARE(load.config.snapshot.faultPressure, 144);
    QCOMPARE(load.config.snapshot.updateIntervalMs, 1700);
    QVERIFY(!load.config.logViewPreferences.showTimestamp);
    QVERIFY(load.config.logViewPreferences.showSource);
    QVERIFY(load.config.logViewPreferences.showLevel);
    QVERIFY(load.repaired);
    QVERIFY(load.reason.contains("logPage.showSource"));
    QVERIFY(load.reason.contains("logPage.showLevel"));
}

QTEST_APPLESS_MAIN(SettingsFileStoreTest)
#include "settings_file_store_test.moc"
