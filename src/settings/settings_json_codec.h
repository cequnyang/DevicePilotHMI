#pragma once

#include <QJsonObject>
#include <QString>

#include "settings/settings_defined.h"

namespace Settings::JsonCodec {
using Settings::PersistedConfig;

struct DecodeResult
{
    bool ok{false};
    PersistedConfig config{};
    bool repaired{false};
    QString reason;
};

inline constexpr int kSchemaVersion{1};
QJsonObject configToJson(const PersistedConfig &config);
DecodeResult jsonToConfig(const QJsonObject &obj);

QByteArray encodeConfig(const PersistedConfig &config);
DecodeResult decodeConfig(const QByteArray &raw);
} // namespace Settings::JsonCodec
