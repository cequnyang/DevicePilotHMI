#pragma once
#include <QString>

#include "settings/settings_defined.h"

namespace Settings::Store {
using Settings::PersistedConfig;

struct PersistResult
{
    bool ok{false};
    QString reason;

    explicit operator bool() const { return ok; }
};
struct LoadResult
{
    PersistedConfig config{};
    bool repaired{false};
    QString reason;
};

QString configFilePath();
PersistResult persistConfig(const PersistedConfig &config);
LoadResult loadConfig();
} // namespace Settings::Store
