#pragma once

namespace Settings {
struct Snapshot
{
    int warningTemperature;
    int faultTemperature;
    int warningPressure;
    int faultPressure;
    int updateIntervalMs;
    bool operator==(const Snapshot &opt) const
    {
        return warningTemperature == opt.warningTemperature
               && faultTemperature == opt.faultTemperature && warningPressure == opt.warningPressure
               && faultPressure == opt.faultPressure && updateIntervalMs == opt.updateIntervalMs;
    }
    bool operator!=(const Snapshot &opt) const { return !(*this == opt); }
};

namespace Default {
inline constexpr int kWarningTemperature = 75;
inline constexpr int kFaultTemperature = 95;
inline constexpr int kWarningPressure = 115;
inline constexpr int kFaultPressure = 135;
inline constexpr int kUpdateIntervalMs = 1000;
} // namespace Default

namespace HighThreshold {
inline constexpr int kWarningTemperature = 199;
inline constexpr int kFaultTemperature = 200;
inline constexpr int kWarningPressure = 149;
inline constexpr int kFaultPressure = 150;
inline constexpr int kUpdateIntervalMs = 5000;
} // namespace HighThreshold

constexpr Snapshot kDefaultSnapshot{Default::kWarningTemperature,
                                    Default::kFaultTemperature,
                                    Default::kWarningPressure,
                                    Default::kFaultPressure,
                                    Default::kUpdateIntervalMs};
constexpr Snapshot defaults()
{
    return kDefaultSnapshot;
}
} // namespace Settings
