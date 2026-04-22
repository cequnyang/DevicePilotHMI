#include "settings/settings_draft.h"

#include "settings/settings_manager.h"
#include "settings/settings_validation.h"

SettingsDraft::SettingsDraft(QObject *parent)
    : QObject(parent)
{
    revalidate();
}

int SettingsDraft::warningTemperature() const
{
    return m_snapshot.warningTemperature;
}

int SettingsDraft::faultTemperature() const
{
    return m_snapshot.faultTemperature;
}

int SettingsDraft::warningPressure() const
{
    return m_snapshot.warningPressure;
}

int SettingsDraft::faultPressure() const
{
    return m_snapshot.faultPressure;
}

int SettingsDraft::updateIntervalMs() const
{
    return m_snapshot.updateIntervalMs;
}

QString SettingsDraft::validationMessage() const
{
    return m_validationMessage;
}

bool SettingsDraft::dirty() const
{
    return m_snapshot != m_originalSnapshot;
}

bool SettingsDraft::canApply() const
{
    return dirty() && valid();
}

void SettingsDraft::setWarningTemperature(int value)
{
    if (m_snapshot.warningTemperature == value) {
        return;
    }
    if (m_snapshot.faultTemperature <= value) {
        m_validationMessage = "Warning temperature must be lower than fault temperature.";
        m_invalidValueSetAttempted = true;
        m_snapshot.warningTemperature = m_snapshot.faultTemperature - 1;
    } else {
        m_invalidValueSetAttempted = false;
        m_snapshot.warningTemperature = value;
    }
    emit warningTemperatureChanged();
    revalidate();
    emit stateChanged();
}

void SettingsDraft::setFaultTemperature(int value)
{
    if (m_snapshot.faultTemperature == value) {
        return;
    }
    if (value <= m_snapshot.warningTemperature) {
        m_validationMessage = "Warning temperature must be lower than fault temperature.";
        m_invalidValueSetAttempted = true;
        m_snapshot.faultTemperature = m_snapshot.warningTemperature + 1;
    } else {
        m_invalidValueSetAttempted = false;
        m_snapshot.faultTemperature = value;
    }
    emit faultTemperatureChanged();
    revalidate();
    emit stateChanged();
}

void SettingsDraft::setWarningPressure(int value)
{
    if (m_snapshot.warningPressure == value) {
        return;
    }
    if (m_snapshot.faultPressure <= value) {
        m_validationMessage = "Warning pressure must be lower than fault pressure.";
        m_invalidValueSetAttempted = true;
        m_snapshot.warningPressure = m_snapshot.faultPressure - 1;
    } else {
        m_invalidValueSetAttempted = false;
        m_snapshot.warningPressure = value;
    }
    emit warningPressureChanged();
    revalidate();
    emit stateChanged();
}

void SettingsDraft::setFaultPressure(int value)
{
    if (m_snapshot.faultPressure == value) {
        return;
    }
    if (value <= m_snapshot.warningPressure) {
        m_validationMessage = "Warning pressure must be lower than fault pressure.";
        m_invalidValueSetAttempted = true;
        m_snapshot.faultPressure = m_snapshot.warningPressure + 1;
    } else {
        m_invalidValueSetAttempted = false;
        m_snapshot.faultPressure = value;
    }
    emit faultPressureChanged();
    revalidate();
    emit stateChanged();
}

void SettingsDraft::setUpdateIntervalMs(int value)
{
    if (m_snapshot.updateIntervalMs == value) {
        return;
    }

    m_snapshot.updateIntervalMs = value;
    emit updateIntervalMsChanged();
    revalidate();
    emit stateChanged();
}

void SettingsDraft::loadFrom(SettingsManager *settings)
{
    if (!settings) {
        return;
    }

    const auto &snapshot = settings->snapshot();
    const auto old = m_snapshot;
    m_snapshot = snapshot;
    m_originalSnapshot = snapshot;
    m_invalidValueSetAttempted = false;
    emitDraftChange(old);
}

void SettingsDraft::loadSnapshot(const Snapshot &snapshot)
{
    if (m_snapshot == snapshot) {
        m_invalidValueSetAttempted = false;
        revalidate();
        emit stateChanged();
        return;
    }

    const auto old = m_snapshot;
    m_snapshot = snapshot;
    m_invalidValueSetAttempted = false;
    emitDraftChange(old);
}

void SettingsDraft::resetDraftToDefaults()
{
    const auto defaults = Settings::defaults();
    if (m_snapshot == defaults) {
        emit stateChanged();
        return;
    }

    const auto old = m_snapshot;
    m_snapshot = defaults;
    m_invalidValueSetAttempted = false;
    emitDraftChange(old);
}

const Snapshot &SettingsDraft::snapshot() const
{
    return m_snapshot;
}

bool SettingsDraft::valid() const
{
    return m_valid;
}

void SettingsDraft::emitDraftChange(const Snapshot &old)
{
    if (old.warningTemperature != m_snapshot.warningTemperature) {
        emit warningTemperatureChanged();
    }
    if (old.faultTemperature != m_snapshot.faultTemperature) {
        emit faultTemperatureChanged();
    }
    if (old.warningPressure != m_snapshot.warningPressure) {
        emit warningPressureChanged();
    }
    if (old.faultPressure != m_snapshot.faultPressure) {
        emit faultPressureChanged();
    }
    if (old.updateIntervalMs != m_snapshot.updateIntervalMs) {
        emit updateIntervalMsChanged();
    }

    revalidate();
    emit stateChanged();
}

void SettingsDraft::revalidate()
{
    auto [valid, validationMessage] = Settings::validateSnapshot(m_snapshot);
    m_valid = valid;
    if (!m_invalidValueSetAttempted || !validationMessage.isEmpty()) {
        m_validationMessage = validationMessage;
    }
}
