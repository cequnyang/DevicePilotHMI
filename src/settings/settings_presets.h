#pragma once

#include <QString>

#include "settings/settings_defined.h"

namespace Settings::Presets {
enum class ThresholdPresetId
{
    Conservative,
    Balanced,
    Aggressive,
};

Snapshot thresholdPresetSnapshot(ThresholdPresetId id, int updateIntervalMs);
QString thresholdPresetName(ThresholdPresetId id);
bool matchesThresholdPreset(const Snapshot &snapshot, ThresholdPresetId id);
} // namespace Settings::Presets
