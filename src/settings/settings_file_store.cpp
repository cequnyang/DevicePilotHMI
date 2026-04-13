#include "settings/settings_file_store.h"

#include <QDir>
#include <QIODevice>
#include <QSaveFile>
#include <QStandardPaths>

#include <cmath>

#include "settings/settings_json_codec.h"
#include "settings/settings_validation.h"

QString Settings::Store::configFilePath()
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(dir);
    return dir + "/devicepilothmi_settings.json";
}

Settings::Store::PersistResult Settings::Store::persistSnapshot(const Snapshot &snapshot)
{
    QSaveFile file(configFilePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return {false, file.errorString()};
    }

    if (file.write(Settings::JsonCodec::encodeSnapshot(snapshot)) == -1) {
        file.cancelWriting();
        return {false, file.errorString()};
    }

    if (!file.commit()) {
        return {false, file.errorString()};
    }

    return {true, {}};
}

Settings::Store::LoadResult Settings::Store::loadSnapshot()
{
    const QString path = configFilePath();
    QFile file(path);

    if (!file.exists()) {
        return {Settings::defaults(), true, "settings.json does not exist. Using defaults."};
    }

    if (!file.open(QIODevice::ReadOnly)) {
        return {Settings::defaults(),
                true,
                QString("Failed to open settings.json (%1). Using defaults.")
                    .arg(file.errorString())};
    }

    const QByteArray raw = file.readAll();
    file.close();

    const auto [codec, loaded, decodeReason] = Settings::JsonCodec::decodeSnapshot(raw);
    if (!codec) {
        return {Settings::defaults(),
                true,
                QString("Invalid settings.json structure (%1). Using defaults.").arg(decodeReason)};
    }

    const auto [validation, validationReason] = Settings::validateSnapshot(loaded);
    if (!validation) {
        return {Settings::defaults(),
                true,
                QString("Invalid settings values (%1). Using defaults.").arg(validationReason)};
    }

    return {loaded, false, {}};
}