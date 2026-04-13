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
    Settings::Snapshot snapshot = Settings::defaults();
    snapshot.warningTemperature = 90;
    snapshot.faultTemperature = 120;
    snapshot.warningPressure = 130;
    snapshot.faultPressure = 145;
    snapshot.updateIntervalMs = 1500;

    const auto persist = Settings::Store::persistSnapshot(snapshot);
    QVERIFY(persist.ok);
    QVERIFY(persist.reason.isEmpty());
    QVERIFY(QFileInfo::exists(Settings::Store::configFilePath()));

    const auto load = Settings::Store::loadSnapshot();
    QCOMPARE(load.snapshot, snapshot);
    QVERIFY(!load.repaired);
    QVERIFY(load.reason.isEmpty());
}

void SettingsFileStoreTest::missingFileReturnsDefaultsAndMarksRepaired()
{
    QFile::remove(Settings::Store::configFilePath());

    const auto load = Settings::Store::loadSnapshot();

    QCOMPARE(load.snapshot, Settings::defaults());
    QVERIFY(load.repaired);
    QVERIFY(load.reason.contains("does not exist", Qt::CaseInsensitive));
}

void SettingsFileStoreTest::invalidJsonReturnsDefaultsAndMarksRepaired()
{
    writeRawSettingsFile("{ definitely-not-json");

    const auto load = Settings::Store::loadSnapshot();

    QCOMPARE(load.snapshot, Settings::defaults());
    QVERIFY(load.repaired);
    QVERIFY(load.reason.contains("Invalid settings.json structure", Qt::CaseInsensitive));
    QVERIFY(load.reason.contains("Invalid JSON", Qt::CaseInsensitive));
}

void SettingsFileStoreTest::nonObjectRootReturnsDefaultsAndMarksRepaired()
{
    writeRawSettingsFile("[1, 2, 3]");

    const auto load = Settings::Store::loadSnapshot();

    QCOMPARE(load.snapshot, Settings::defaults());
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
        "updateIntervalMs": 1000
    })");

    const auto load = Settings::Store::loadSnapshot();

    QCOMPARE(load.snapshot, Settings::defaults());
    QVERIFY(load.repaired);
    QVERIFY(load.reason.contains("Invalid settings values", Qt::CaseInsensitive));
    QVERIFY(load.reason.contains("Warning temperature must be lower than fault temperature.",
                                 Qt::CaseInsensitive));
}

QTEST_APPLESS_MAIN(SettingsFileStoreTest)
#include "settings_file_store_test.moc"
