#include "settings/settings_apply_service.h"

#include "log/log_event.h"
#include "log/log_interface.h"
#include "runtime/machine_runtime.h"
#include "settings/settings_manager.h"
#include "settings/settings_validation.h"

SettingsApplyService::SettingsApplyService(LogInterface &logInterface,
                                           SettingsManager &settings,
                                           MachineRuntime &machineRuntime,
                                           QObject *parent)
    : QObject(parent)
    , m_logInterface(&logInterface)
    , m_settings(&settings)
    , m_machineRuntime(&machineRuntime)

{
    Q_ASSERT(m_logInterface);
    Q_ASSERT(m_settings);
    Q_ASSERT(m_machineRuntime);

    appendLog(LogEvent{
        .level = "INFO",
        .source = "settings",
        .eventType = "settings.initialize.succeeded",
        .message = "Settings Apply Service initialized",
    });

    connect(m_machineRuntime,
            &MachineRuntime::stateChanged,
            this,
            &SettingsApplyService::policyContextChanged);
}

bool SettingsApplyService::applySettings(const Snapshot &candidate)
{
    const SettingsApplyAnalysis analysis = analyzeSettingsApply(candidate);
    if (!analysis.allowed) {
        appendLog(LogEvent{
            .level = "CONFIG",
            .source = "settings",
            .eventType = "settings.apply.rejected",
            .message = QString("Rejected settings apply (%1)").arg(analysis.reason),
        });

        return false;
    }
    const auto [apply, reason] = m_settings->applySnapshot(candidate);
    if (!apply) {
        appendLog(LogEvent{
            .level = "CONFIG",
            .source = "settings",
            .eventType = "settings.apply.rejected",
            .message = QString("Rejected settings apply (%1)").arg(reason),
        });

        return false;
    }

    appendLog(LogEvent{
        .level = "CONFIG",
        .source = "settings",
        .eventType = "settings.apply.succeeded",
        .message = "Settings applied",
    });

    return true;
}

QString SettingsApplyService::settingsApplyRestrictionReason(const Snapshot &candidate) const
{
    return analyzeSettingsApply(candidate).reason;
}

SettingsApplyAnalysis SettingsApplyService::analyzeSettingsApply(const Snapshot &candidate) const
{
    SettingsApplyAnalysis result;

    const auto [validation, reason] = Settings::validateSnapshot(candidate);
    if (!validation) {
        result.reason = reason;
        result.allowed = false;
        return result;
    }

    const Snapshot &committed = m_settings->snapshot();

    const bool warningTempChanged = candidate.warningTemperature != committed.warningTemperature;
    const bool faultTempChanged = candidate.faultTemperature != committed.faultTemperature;
    const bool warningPressureChanged = candidate.warningPressure != committed.warningPressure;
    const bool faultPressureChanged = candidate.faultPressure != committed.faultPressure;
    const bool updateIntervalChanged = candidate.updateIntervalMs != committed.updateIntervalMs;

    result.changesThresholds = warningTempChanged || faultTempChanged || warningPressureChanged
                               || faultPressureChanged;
    result.changesUpdateInterval = updateIntervalChanged;

    if (!result.changesThresholds && !result.changesUpdateInterval) {
        result.allowed = true;
        return result;
    }

    switch (m_machineRuntime->state()) {
    case MachineRuntime::State::Idle:
        result.allowed = true;
        return result;

    case MachineRuntime::State::Running:
        if (result.changesUpdateInterval) {
            result.allowed = false;
            result.reason
                = "Update interval changes can only be applied while the machine is idle.";
            return result;
        }
        result.allowed = true;
        return result;

    case MachineRuntime::State::Starting:
        result.allowed = false;
        result.reason = "Settings cannot be applied while the machine is starting.";
        return result;

    case MachineRuntime::State::Stopping:
        result.allowed = false;
        result.reason = "Settings cannot be applied while the machine is stopping.";
        return result;

    case MachineRuntime::State::Fault:
        result.allowed = false;
        result.reason
            = "Settings cannot be applied while the machine is in fault state. Reset fault first.";
        return result;
    }

    result.allowed = false;
    result.reason = "Unknown machine state.";
    return result;
}

void SettingsApplyService::appendLog(const LogEvent &event)
{
    m_logInterface->appendLog(event);
}