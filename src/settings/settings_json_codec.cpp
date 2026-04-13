#include "settings/settings_json_codec.h"

#include <QJsonDocument>

QJsonObject Settings::JsonCodec::snapshotToJson(const Snapshot &snapshot)
{
    return {{"schemaVersion", kSchemaVersion},
            {"warningTemperature", snapshot.warningTemperature},
            {"faultTemperature", snapshot.faultTemperature},
            {"warningPressure", snapshot.warningPressure},
            {"faultPressure", snapshot.faultPressure},
            {"updateIntervalMs", snapshot.updateIntervalMs}};
}

Settings::JsonCodec::DecodeResult Settings::JsonCodec::jsonToSnapshot(const QJsonObject &obj)
{
    if (!obj.contains("schemaVersion")) {
        return {false, {}, "Missing field 'schemaVersion'."};
    }

    const QJsonValue schemaValue = obj.value("schemaVersion");
    if (!schemaValue.isDouble() || schemaValue.toInt(-1) != kSchemaVersion) {
        return {false,
                {},
                QString("Unsupported schemaVersion. Expected %1.")
                    .arg(QString::number(kSchemaVersion))};
    }

    QString reason;
    auto readIntField = [&obj, &reason](const char *key, int *out) -> bool {
        if (!obj.contains(key)) {
            reason = QString("Missing field '%1'.").arg(QString::fromLatin1(key));
            return false;
        }

        const QJsonValue value = obj.value(key);
        if (!value.isDouble()) {
            reason = QString("Field '%1' is not a number.").arg(QString::fromLatin1(key));
            return false;
        }

        const double raw = value.toDouble();
        if (!std::isfinite(raw) || std::floor(raw) != raw) {
            reason = QString("Field '%1' must be an integer.").arg(QString::fromLatin1(key));
            return false;
        }

        *out = static_cast<int>(raw);
        return true;
    };

    Snapshot parsed{};
    if (!readIntField("warningTemperature", &parsed.warningTemperature)
        || !readIntField("faultTemperature", &parsed.faultTemperature)
        || !readIntField("warningPressure", &parsed.warningPressure)
        || !readIntField("faultPressure", &parsed.faultPressure)
        || !readIntField("updateIntervalMs", &parsed.updateIntervalMs)) {
        return {false, parsed, reason};
    }

    return {true, parsed, reason};
}

QByteArray Settings::JsonCodec::encodeSnapshot(const Snapshot &snapshot)
{
    const QJsonDocument doc(Settings::JsonCodec::snapshotToJson(snapshot));
    return doc.toJson(QJsonDocument::Indented);
}

Settings::JsonCodec::DecodeResult Settings::JsonCodec::decodeSnapshot(const QByteArray &raw)
{
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(raw, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        return {false,
                Settings::defaults(),
                QString("Invalid JSON in settings.json (%1). Using defaults.")
                    .arg(parseError.errorString())};
    }

    if (!doc.isObject()) {
        return {false,
                Settings::defaults(),
                "settings.json root is not a JSON object. Using defaults."};
    }
    return jsonToSnapshot(doc.object());
}