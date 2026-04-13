#pragma once

#include "settings/settings_defined.h"

#include <QString>

namespace Settings {
using Settings::Snapshot;

struct ValidationResult
{
    bool ok{false};
    QString reason;

    explicit operator bool() const { return ok; }
};

ValidationResult validateSnapshot(const Snapshot &snapshot);
} // namespace Settings