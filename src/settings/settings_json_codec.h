#pragma once

#include <QJsonObject>
#include <QString>

#include "settings/settings_defined.h"

namespace Settings::JsonCodec {
using Settings::Snapshot;

struct DecodeResult
{
    bool ok{false};
    Snapshot snapshot{};
    QString reason;
};

inline constexpr int kSchemaVersion{1};
QJsonObject snapshotToJson(const Snapshot &snapshot);
DecodeResult jsonToSnapshot(const QJsonObject &obj);

QByteArray encodeSnapshot(const Snapshot &snapshot);
DecodeResult decodeSnapshot(const QByteArray &raw);
} // namespace Settings::JsonCodec