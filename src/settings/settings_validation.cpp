#include "settings/settings_validation.h"

Settings::ValidationResult Settings::validateSnapshot(const Snapshot &snapshot)
{
    if (snapshot.warningTemperature < 0
        || Settings::HighThreshold::kWarningTemperature < snapshot.warningTemperature) {
        return {false,
                "Warning temperature must be between 0 and "
                    + QString::number(Settings::HighThreshold::kWarningTemperature) + "."};
    }

    if (snapshot.faultTemperature < 1
        || Settings::HighThreshold::kFaultTemperature < snapshot.faultTemperature) {
        return {false,
                "Fault temperature must be between 1 and "
                    + QString::number(Settings::HighThreshold::kFaultTemperature) + "."};
    }

    if (snapshot.warningPressure < 0
        || Settings::HighThreshold::kWarningPressure < snapshot.warningPressure) {
        return {false,
                "Warning pressure must be between 0 and "
                    + QString::number(Settings::HighThreshold::kWarningPressure) + "."};
    }

    if (snapshot.faultPressure < 1
        || Settings::HighThreshold::kFaultPressure < snapshot.faultPressure) {
        return {false,
                "Fault pressure must be between 1 and "
                    + QString::number(Settings::HighThreshold::kFaultPressure) + "."};
    }

    if (snapshot.updateIntervalMs < 100
        || Settings::HighThreshold::kUpdateIntervalMs < snapshot.updateIntervalMs) {
        return {false,
                "Update interval must be between 100 and "
                    + QString::number(Settings::HighThreshold::kUpdateIntervalMs) + " ms."};
    }

    if (snapshot.faultTemperature <= snapshot.warningTemperature) {
        return {false, "Warning temperature must be lower than fault temperature."};
    }

    if (snapshot.faultPressure <= snapshot.warningPressure) {
        return {false, "Warning pressure must be lower than fault pressure."};
    }
    return {true, {}};
}