#include "settings/settings_json_codec.h"

#include <QJsonDocument>

#include <cmath>
#include <QStringList>

namespace {
QJsonObject snapshotToJson(const Settings::Snapshot &snapshot)
{
    return {{"schemaVersion", Settings::JsonCodec::kSchemaVersion},
            {"warningTemperature", snapshot.warningTemperature},
            {"faultTemperature", snapshot.faultTemperature},
            {"warningPressure", snapshot.warningPressure},
            {"faultPressure", snapshot.faultPressure},
            {"updateIntervalMs", snapshot.updateIntervalMs}};
}

QJsonObject logViewPreferencesToJson(const Settings::LogViewPreferences &preferences)
{
    return {{"showTimestamp", preferences.showTimestamp},
            {"showSource", preferences.showSource},
            {"showLevel", preferences.showLevel}};
}
} // namespace

QJsonObject Settings::JsonCodec::configToJson(const PersistedConfig &config)
{
    QJsonObject object = snapshotToJson(config.snapshot);
    object.insert("logPage", logViewPreferencesToJson(config.logViewPreferences));
    return object;
}

Settings::JsonCodec::DecodeResult Settings::JsonCodec::jsonToConfig(const QJsonObject &obj)
{
    if (!obj.contains("schemaVersion")) {
        return {false, Settings::defaultsConfig(), false, "Missing field 'schemaVersion'."};
    }

    const QJsonValue schemaValue = obj.value("schemaVersion");
    if (!schemaValue.isDouble() || schemaValue.toInt(-1) != kSchemaVersion) {
        return {false,
                Settings::defaultsConfig(),
                false,
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

    Settings::Snapshot parsed{};
    if (!readIntField("warningTemperature", &parsed.warningTemperature)
        || !readIntField("faultTemperature", &parsed.faultTemperature)
        || !readIntField("warningPressure", &parsed.warningPressure)
        || !readIntField("faultPressure", &parsed.faultPressure)
        || !readIntField("updateIntervalMs", &parsed.updateIntervalMs)) {
        return {false, Settings::defaultsConfig(), false, reason};
    }

    Settings::LogViewPreferences logViewPreferences = Settings::defaultLogViewPreferences();
    bool repaired = false;
    QStringList repairReasons;

    if (!obj.contains("logPage")) {
        repaired = true;
        repairReasons << "Missing field 'logPage'. Using defaults.";
    } else {
        const QJsonValue logPageValue = obj.value("logPage");
        if (!logPageValue.isObject()) {
            repaired = true;
            repairReasons << "Field 'logPage' is not an object. Using defaults.";
        } else {
            const QJsonObject logPageObject = logPageValue.toObject();
            const auto readBoolField = [&logPageObject, &repaired, &repairReasons](const char *key,
                                                                                    bool *out)
                -> void {
                if (!logPageObject.contains(key)) {
                    repaired = true;
                    repairReasons
                        << QString("Missing field 'logPage.%1'. Using default.")
                               .arg(QString::fromLatin1(key));
                    return;
                }

                const QJsonValue value = logPageObject.value(key);
                if (!value.isBool()) {
                    repaired = true;
                    repairReasons
                        << QString("Field 'logPage.%1' is not a boolean. Using default.")
                               .arg(QString::fromLatin1(key));
                    return;
                }

                *out = value.toBool();
            };

            readBoolField("showTimestamp", &logViewPreferences.showTimestamp);
            readBoolField("showSource", &logViewPreferences.showSource);
            readBoolField("showLevel", &logViewPreferences.showLevel);
        }
    }

    return {true,
            Settings::PersistedConfig{parsed, logViewPreferences},
            repaired,
            repairReasons.join(' ')};
}

QByteArray Settings::JsonCodec::encodeConfig(const PersistedConfig &config)
{
    const QJsonDocument doc(Settings::JsonCodec::configToJson(config));
    return doc.toJson(QJsonDocument::Indented);
}

Settings::JsonCodec::DecodeResult Settings::JsonCodec::decodeConfig(const QByteArray &raw)
{
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(raw, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        return {false,
                Settings::defaultsConfig(),
                false,
                QString("Invalid JSON in settings.json (%1). Using defaults.")
                    .arg(parseError.errorString())};
    }

    if (!doc.isObject()) {
        return {false,
                Settings::defaultsConfig(),
                false,
                "settings.json root is not a JSON object. Using defaults."};
    }
    return jsonToConfig(doc.object());
}
