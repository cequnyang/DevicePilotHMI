#include "settings/settings_presets.h"

namespace {
struct ThresholdPresetValues
{
    int warningTemperature;
    int faultTemperature;
    int warningPressure;
    int faultPressure;
};

constexpr ThresholdPresetValues valuesForPreset(Settings::Presets::ThresholdPresetId id)
{
    using Settings::Presets::ThresholdPresetId;

    switch (id) {
    case ThresholdPresetId::Conservative:
        return ThresholdPresetValues{70, 85, 110, 125};
    case ThresholdPresetId::Balanced:
        return ThresholdPresetValues{Settings::Default::kWarningTemperature,
                                     Settings::Default::kFaultTemperature,
                                     Settings::Default::kWarningPressure,
                                     Settings::Default::kFaultPressure};
    case ThresholdPresetId::Aggressive:
        return ThresholdPresetValues{85, 110, 125, 145};
    }

    return ThresholdPresetValues{Settings::Default::kWarningTemperature,
                                 Settings::Default::kFaultTemperature,
                                 Settings::Default::kWarningPressure,
                                 Settings::Default::kFaultPressure};
}
} // namespace

Settings::Snapshot Settings::Presets::thresholdPresetSnapshot(ThresholdPresetId id,
                                                              int updateIntervalMs)
{
    const auto values = valuesForPreset(id);
    return Settings::Snapshot{values.warningTemperature,
                              values.faultTemperature,
                              values.warningPressure,
                              values.faultPressure,
                              updateIntervalMs};
}

QString Settings::Presets::thresholdPresetName(ThresholdPresetId id)
{
    switch (id) {
    case ThresholdPresetId::Conservative:
        return QStringLiteral("Conservative");
    case ThresholdPresetId::Balanced:
        return QStringLiteral("Balanced");
    case ThresholdPresetId::Aggressive:
        return QStringLiteral("Aggressive");
    }

    return QStringLiteral("Balanced");
}

bool Settings::Presets::matchesThresholdPreset(const Snapshot &snapshot, ThresholdPresetId id)
{
    const auto values = valuesForPreset(id);
    return snapshot.warningTemperature == values.warningTemperature
           && snapshot.faultTemperature == values.faultTemperature
           && snapshot.warningPressure == values.warningPressure
           && snapshot.faultPressure == values.faultPressure;
}
