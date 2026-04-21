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

    connect(m_draft, &SettingsDraft::stateChanged, this, &SettingsSession::applyStateChanged);
    connect(m_applyService,
            &SettingsApplyService::policyContextChanged,
            this,
            &SettingsSession::applyStateChanged);
    connect(m_manager, &SettingsManager::thresholdsChanged, this, [this]() {
        emit committedSettingsChanged();
        emit applyStateChanged();
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

bool SettingsSession::apply()
{
    if (!m_applyService->applySettings(snapshotFromDraft())) {
        return false;
    }

    m_draft->loadFrom(m_manager);
    return true;
}

void SettingsSession::reload()
{
    m_draft->loadFrom(m_manager);
}

const Snapshot &SettingsSession::snapshotFromDraft() const
{
    return m_draft->snapshot();
}

void SettingsSession::appendLog(const LogEvent &event)
{
    m_logInterface->appendLog(event);
}
