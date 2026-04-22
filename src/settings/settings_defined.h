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

struct LogViewPreferences
{
    bool showTimestamp{true};
    bool showSource{true};
    bool showLevel{true};

    bool operator==(const LogViewPreferences &opt) const
    {
        return showTimestamp == opt.showTimestamp && showSource == opt.showSource
               && showLevel == opt.showLevel;
    }
    bool operator!=(const LogViewPreferences &opt) const { return !(*this == opt); }
};

namespace Default {
inline constexpr int kWarningTemperature = 75;
inline constexpr int kFaultTemperature = 95;
inline constexpr int kWarningPressure = 115;
inline constexpr int kFaultPressure = 135;
inline constexpr int kUpdateIntervalMs = 1000;
} // namespace Default

namespace LogViewDefault {
inline constexpr bool kShowTimestamp = true;
inline constexpr bool kShowSource = true;
inline constexpr bool kShowLevel = true;
} // namespace LogViewDefault

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
constexpr LogViewPreferences kDefaultLogViewPreferences{LogViewDefault::kShowTimestamp,
                                                        LogViewDefault::kShowSource,
                                                        LogViewDefault::kShowLevel};

struct PersistedConfig
{
    Snapshot snapshot{kDefaultSnapshot};
    LogViewPreferences logViewPreferences{kDefaultLogViewPreferences};

    bool operator==(const PersistedConfig &opt) const
    {
        return snapshot == opt.snapshot && logViewPreferences == opt.logViewPreferences;
    }
    bool operator!=(const PersistedConfig &opt) const { return !(*this == opt); }
};

constexpr PersistedConfig kDefaultPersistedConfig{kDefaultSnapshot, kDefaultLogViewPreferences};

constexpr Snapshot defaults()
{
    return kDefaultSnapshot;
}

constexpr LogViewPreferences defaultLogViewPreferences()
{
    return kDefaultLogViewPreferences;
}

constexpr PersistedConfig defaultsConfig()
{
    return kDefaultPersistedConfig;
}
} // namespace Settings
