#include "settings/settings_session.h"

#include "log/log_event.h"
#include "log/log_interface.h"
#include "settings/settings_apply_service.h"
#include "settings/settings_manager.h"

SettingsSession::SettingsSession(LogInterface &logInterface,
                                 SettingsManager &settingsManager,
                                 SettingsApplyService &settingsApplyService,
                                 QObject *parent)
    : QObject(parent)
    , m_logInterface(&logInterface)
    , m_manager(&settingsManager)
    , m_applyService(&settingsApplyService)
    , m_draft(new SettingsDraft(this))

{
    Q_ASSERT(m_logInterface);
    Q_ASSERT(m_manager);
    Q_ASSERT(m_applyService);
    Q_ASSERT(m_draft);

    appendLog(LogEvent{
        .level = "INFO",
        .source = "settings",
        .eventType = "settings.session.initialized",
        .message = "Settings Session initialized",
    });

    connect(m_draft, &SettingsDraft::stateChanged, this, [this]() {
        emit applyStateChanged();
        emit comparisonStateChanged();
    });
    connect(m_applyService,
            &SettingsApplyService::policyContextChanged,
            this,
            &SettingsSession::applyStateChanged);
    connect(m_manager, &SettingsManager::thresholdsChanged, this, [this]() {
        emit committedSettingsChanged();
        emit applyStateChanged();
        emit comparisonStateChanged();
    });
    connect(m_manager, &SettingsManager::updateIntervalMsChanged, this, [this]() {
        emit committedSettingsChanged();
        emit applyStateChanged();
        emit comparisonStateChanged();
    });
    reload();
}

SettingsDraft *SettingsSession::draft() const
{
    return m_draft;
}

bool SettingsSession::applyEnabled() const
{
    return m_draft->canApply() && applyRestrictionReason().isEmpty();
}

QString SettingsSession::applyRestrictionReason() const
{
    if (!m_draft->dirty() || !m_draft->valid()) {
        return "";
    }

    return m_applyService->settingsApplyRestrictionReason(snapshotFromDraft());
}

QString SettingsSession::committedThresholdPresetName() const
{
    return thresholdPresetNameForSnapshot(m_manager->snapshot());
}

QString SettingsSession::draftThresholdPresetName() const
{
    return thresholdPresetNameForSnapshot(snapshotFromDraft());
}

int SettingsSession::pendingChangeCount() const
{
    const Snapshot &draft = snapshotFromDraft();
    const Snapshot &committed = m_manager->snapshot();

    int count = 0;
    count += static_cast<int>(draft.warningTemperature != committed.warningTemperature);
    count += static_cast<int>(draft.faultTemperature != committed.faultTemperature);
    count += static_cast<int>(draft.warningPressure != committed.warningPressure);
    count += static_cast<int>(draft.faultPressure != committed.faultPressure);
    count += static_cast<int>(draft.updateIntervalMs != committed.updateIntervalMs);
    return count;
}

int SettingsSession::committedWarningTemperature() const
{
    return m_manager->snapshot().warningTemperature;
}

int SettingsSession::committedFaultTemperature() const
{
    return m_manager->snapshot().faultTemperature;
}

int SettingsSession::committedWarningPressure() const
{
    return m_manager->snapshot().warningPressure;
}

int SettingsSession::committedFaultPressure() const
{
    return m_manager->snapshot().faultPressure;
}

int SettingsSession::committedUpdateIntervalMs() const
{
    return m_manager->snapshot().updateIntervalMs;
}

bool SettingsSession::apply()
{
    if (!m_applyService->applySettings(snapshotFromDraft())) {
        return false;
    }

    m_draft->loadFrom(m_manager);
    return true;
}

void SettingsSession::loadConservativePreset()
{
    loadThresholdPreset(Settings::Presets::ThresholdPresetId::Conservative);
}

void SettingsSession::loadBalancedPreset()
{
    loadThresholdPreset(Settings::Presets::ThresholdPresetId::Balanced);
}

void SettingsSession::loadAggressivePreset()
{
    loadThresholdPreset(Settings::Presets::ThresholdPresetId::Aggressive);
}

void SettingsSession::reload()
{
    m_draft->loadFrom(m_manager);
}

const Snapshot &SettingsSession::snapshotFromDraft() const
{
    return m_draft->snapshot();
}

QString SettingsSession::thresholdPresetNameForSnapshot(const Snapshot &snapshot) const
{
    using Settings::Presets::ThresholdPresetId;

    constexpr std::array<ThresholdPresetId, 3> kPresets = {{
        ThresholdPresetId::Conservative,
        ThresholdPresetId::Balanced,
        ThresholdPresetId::Aggressive,
    }};

    for (const auto presetItem : kPresets) {
        if (Settings::Presets::matchesThresholdPreset(snapshot, presetItem)) {
            return Settings::Presets::thresholdPresetName(presetItem);
        }
    }

    return QStringLiteral("Custom");
}

void SettingsSession::loadThresholdPreset(Settings::Presets::ThresholdPresetId presetId)
{
    m_draft->loadSnapshot(
        Settings::Presets::thresholdPresetSnapshot(presetId, m_draft->updateIntervalMs()));
}

void SettingsSession::appendLog(const LogEvent &event)
{
    m_logInterface->appendLog(event);
}
