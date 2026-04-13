#pragma once
#include <QString>

#include "settings/settings_defined.h"

namespace Settings::Store {
using Settings::Snapshot;

struct PersistResult
{
    bool ok{false};
    QString reason;

    explicit operator bool() const { return ok; }
};
struct LoadResult
{
    Snapshot snapshot{};
    bool repaired{false};
    QString reason;
};

QString configFilePath();
PersistResult persistSnapshot(const Snapshot &snapshot);
LoadResult loadSnapshot();
} // namespace Settings::Store