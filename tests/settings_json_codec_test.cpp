#include <QtTest>

#include "settings/settings_defined.h"
#include "settings/settings_json_codec.h"

class SettingsJsonCodecTest : public QObject
{
    Q_OBJECT

private slots:
    void jsonAndSnapshotRoundTrip();
    void encodeAndDecodeRoundTrip();
    void jsonToSnapshotRejectsMissingSchemaVersion();
    void jsonToSnapshotRejectsMissingKey();
    void jsonToSnapshotRejectsInvalidValue();
    void decodeSnapshotRejectsInvalidJson();
    void decodeSnapshotRejectsNonObjectRoot();
};

void SettingsJsonCodecTest::jsonAndSnapshotRoundTrip()
{
    Settings::Snapshot snapshot = Settings::defaults();
    snapshot.warningTemperature = 90;
    snapshot.faultTemperature = 140;
    snapshot.warningPressure = 120;
    snapshot.faultPressure = 145;
    snapshot.updateIntervalMs = 1500;

    const auto json = Settings::JsonCodec::snapshotToJson(snapshot);
    const auto result = Settings::JsonCodec::jsonToSnapshot(json);

    QVERIFY(result.ok);
    QCOMPARE(result.snapshot, snapshot);
    QVERIFY(result.reason.isEmpty());
}

void SettingsJsonCodecTest::encodeAndDecodeRoundTrip()
{
    Settings::Snapshot snapshot = Settings::defaults();
    snapshot.warningTemperature = 90;
    snapshot.faultTemperature = 140;
    snapshot.warningPressure = 120;
    snapshot.faultPressure = 145;
    snapshot.updateIntervalMs = 1500;

    const auto encoded = Settings::JsonCodec::encodeSnapshot(snapshot);
    const auto result = Settings::JsonCodec::decodeSnapshot(encoded);

    QVERIFY(result.ok);
    QCOMPARE(result.snapshot, snapshot);
    QVERIFY(result.reason.isEmpty());
}

void SettingsJsonCodecTest::jsonToSnapshotRejectsMissingSchemaVersion()
{
    const QJsonObject obj{
        {"warningTemperature", 75},
        {"faultTemperature", 95},
        {"warningPressure", 115},
        {"faultPressure", 135},
        {"updateIntervalMs", 1000},
    };

    const auto result = Settings::JsonCodec::jsonToSnapshot(obj);

    QVERIFY(!result.ok);
    QVERIFY(result.reason.contains("schemaVersion"));
}

void SettingsJsonCodecTest::jsonToSnapshotRejectsMissingKey()
{
    const QJsonObject obj{
        {"schemaVersion", Settings::JsonCodec::kSchemaVersion},
        {"warningTemperature", 75},
        {"faultTemperature", 95},
        {"missingKey", 115},
        {"faultPressure", 135},
        {"updateIntervalMs", 1000},
    };

    const auto result = Settings::JsonCodec::jsonToSnapshot(obj);

    QVERIFY(!result.ok);
    QVERIFY(result.reason.contains("Missing field"));
}

void SettingsJsonCodecTest::jsonToSnapshotRejectsInvalidValue()
{
    const QJsonObject obj{
        {"schemaVersion", Settings::JsonCodec::kSchemaVersion},
        {"warningTemperature", "75"},
        {"faultTemperature", 95},
        {"warningPressure", 115},
        {"faultPressure", 135},
        {"updateIntervalMs", 1000},
    };

    const auto result = Settings::JsonCodec::jsonToSnapshot(obj);

    QVERIFY(!result.ok);
    QVERIFY(result.reason.contains("is not a number"));
}

void SettingsJsonCodecTest::decodeSnapshotRejectsInvalidJson()
{
    const auto result = Settings::JsonCodec::decodeSnapshot("{ definitely-not-json");

    QVERIFY(!result.ok);
    QCOMPARE(result.snapshot, Settings::defaults());
    QVERIFY(result.reason.contains("Invalid JSON"));
}

void SettingsJsonCodecTest::decodeSnapshotRejectsNonObjectRoot()
{
    const auto result = Settings::JsonCodec::decodeSnapshot("[1, 2, 3]");

    QVERIFY(!result.ok);
    QCOMPARE(result.snapshot, Settings::defaults());
    QVERIFY(result.reason.contains("not a JSON object"));
}

QTEST_APPLESS_MAIN(SettingsJsonCodecTest)
#include "settings_json_codec_test.moc"
