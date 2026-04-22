#include <QtTest>

#include "settings/settings_defined.h"
#include "settings/settings_json_codec.h"

class SettingsJsonCodecTest : public QObject
{
    Q_OBJECT

private slots:
    void jsonAndConfigRoundTrip();
    void encodeAndDecodeRoundTrip();
    void jsonToConfigRepairsMissingLogPage();
    void jsonToConfigRepairsInvalidLogPageFields();
    void jsonToConfigRejectsMissingSchemaVersion();
    void jsonToConfigRejectsMissingSnapshotKey();
    void jsonToConfigRejectsInvalidSnapshotValue();
    void decodeConfigRejectsInvalidJson();
    void decodeConfigRejectsNonObjectRoot();
};

void SettingsJsonCodecTest::jsonAndConfigRoundTrip()
{
    Settings::PersistedConfig config = Settings::defaultsConfig();
    config.snapshot.warningTemperature = 90;
    config.snapshot.faultTemperature = 140;
    config.snapshot.warningPressure = 120;
    config.snapshot.faultPressure = 145;
    config.snapshot.updateIntervalMs = 1500;
    config.logViewPreferences.showTimestamp = false;
    config.logViewPreferences.showSource = false;

    const auto json = Settings::JsonCodec::configToJson(config);
    const auto result = Settings::JsonCodec::jsonToConfig(json);

    QVERIFY(result.ok);
    QCOMPARE(result.config, config);
    QVERIFY(!result.repaired);
    QVERIFY(result.reason.isEmpty());
}

void SettingsJsonCodecTest::encodeAndDecodeRoundTrip()
{
    Settings::PersistedConfig config = Settings::defaultsConfig();
    config.snapshot.warningTemperature = 90;
    config.snapshot.faultTemperature = 140;
    config.snapshot.warningPressure = 120;
    config.snapshot.faultPressure = 145;
    config.snapshot.updateIntervalMs = 1500;
    config.logViewPreferences.showTimestamp = false;
    config.logViewPreferences.showLevel = false;

    const auto encoded = Settings::JsonCodec::encodeConfig(config);
    const auto result = Settings::JsonCodec::decodeConfig(encoded);

    QVERIFY(result.ok);
    QCOMPARE(result.config, config);
    QVERIFY(!result.repaired);
    QVERIFY(result.reason.isEmpty());
}

void SettingsJsonCodecTest::jsonToConfigRepairsMissingLogPage()
{
    const QJsonObject obj{
        {"schemaVersion", Settings::JsonCodec::kSchemaVersion},
        {"warningTemperature", 75},
        {"faultTemperature", 95},
        {"warningPressure", 115},
        {"faultPressure", 135},
        {"updateIntervalMs", 1000},
    };

    const auto result = Settings::JsonCodec::jsonToConfig(obj);

    QVERIFY(result.ok);
    QCOMPARE(result.config.snapshot, Settings::defaults());
    QCOMPARE(result.config.logViewPreferences, Settings::defaultLogViewPreferences());
    QVERIFY(result.repaired);
    QVERIFY(result.reason.contains("logPage"));
}

void SettingsJsonCodecTest::jsonToConfigRepairsInvalidLogPageFields()
{
    const QJsonObject obj{
        {"schemaVersion", Settings::JsonCodec::kSchemaVersion},
        {"warningTemperature", 82},
        {"faultTemperature", 96},
        {"warningPressure", 121},
        {"faultPressure", 141},
        {"updateIntervalMs", 1300},
        {"logPage",
         QJsonObject{
             {"showTimestamp", false},
             {"showSource", 1},
         }},
    };

    const auto result = Settings::JsonCodec::jsonToConfig(obj);

    QVERIFY(result.ok);
    QCOMPARE(result.config.snapshot.warningTemperature, 82);
    QCOMPARE(result.config.snapshot.faultTemperature, 96);
    QCOMPARE(result.config.snapshot.warningPressure, 121);
    QCOMPARE(result.config.snapshot.faultPressure, 141);
    QCOMPARE(result.config.snapshot.updateIntervalMs, 1300);
    QVERIFY(!result.config.logViewPreferences.showTimestamp);
    QVERIFY(result.config.logViewPreferences.showSource);
    QVERIFY(result.config.logViewPreferences.showLevel);
    QVERIFY(result.repaired);
    QVERIFY(result.reason.contains("logPage.showSource"));
    QVERIFY(result.reason.contains("logPage.showLevel"));
}

void SettingsJsonCodecTest::jsonToConfigRejectsMissingSchemaVersion()
{
    const QJsonObject obj{
        {"warningTemperature", 75},
        {"faultTemperature", 95},
        {"warningPressure", 115},
        {"faultPressure", 135},
        {"updateIntervalMs", 1000},
    };

    const auto result = Settings::JsonCodec::jsonToConfig(obj);

    QVERIFY(!result.ok);
    QVERIFY(result.reason.contains("schemaVersion"));
}

void SettingsJsonCodecTest::jsonToConfigRejectsMissingSnapshotKey()
{
    const QJsonObject obj{
        {"schemaVersion", Settings::JsonCodec::kSchemaVersion},
        {"warningTemperature", 75},
        {"faultTemperature", 95},
        {"missingKey", 115},
        {"faultPressure", 135},
        {"updateIntervalMs", 1000},
    };

    const auto result = Settings::JsonCodec::jsonToConfig(obj);

    QVERIFY(!result.ok);
    QVERIFY(result.reason.contains("Missing field"));
}

void SettingsJsonCodecTest::jsonToConfigRejectsInvalidSnapshotValue()
{
    const QJsonObject obj{
        {"schemaVersion", Settings::JsonCodec::kSchemaVersion},
        {"warningTemperature", "75"},
        {"faultTemperature", 95},
        {"warningPressure", 115},
        {"faultPressure", 135},
        {"updateIntervalMs", 1000},
    };

    const auto result = Settings::JsonCodec::jsonToConfig(obj);

    QVERIFY(!result.ok);
    QVERIFY(result.reason.contains("is not a number"));
}

void SettingsJsonCodecTest::decodeConfigRejectsInvalidJson()
{
    const auto result = Settings::JsonCodec::decodeConfig("{ definitely-not-json");

    QVERIFY(!result.ok);
    QCOMPARE(result.config, Settings::defaultsConfig());
    QVERIFY(result.reason.contains("Invalid JSON"));
}

void SettingsJsonCodecTest::decodeConfigRejectsNonObjectRoot()
{
    const auto result = Settings::JsonCodec::decodeConfig("[1, 2, 3]");

    QVERIFY(!result.ok);
    QCOMPARE(result.config, Settings::defaultsConfig());
    QVERIFY(result.reason.contains("not a JSON object"));
}

QTEST_APPLESS_MAIN(SettingsJsonCodecTest)
#include "settings_json_codec_test.moc"
