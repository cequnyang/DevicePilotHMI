#include "backend/simulation_strategy.h"

// Helpers

std::unique_ptr<SimulationStrategy> makeSimulationStrategy(Simulation::Scenario scenario)
{
    switch (scenario) {
    case Simulation::Scenario::NormalRamp:
        return std::make_unique<NormalRampStrategy>();
    case Simulation::Scenario::Overload:
        return std::make_unique<OverloadStrategy>();
    case Simulation::Scenario::CoolingFailure:
        return std::make_unique<CoolingFailureStrategy>();
    case Simulation::Scenario::LoadStepResponse:
        return std::make_unique<LoadStepResponseStrategy>();
    case Simulation::Scenario::Unknown:
        break;
    }

    return std::make_unique<NormalRampStrategy>();
}

namespace {
double secondsPerTick(const Settings::Snapshot &settings)
{
    return std::max(0.001, settings.updateIntervalMs / 1000.0);
}

int stepTowards(int current, int target, int step)
{
    if (current < target) {
        return std::min(target, current + step);
    }
    if (current > target) {
        return std::max(target, current - step);
    }
    return current;
}
} // namespace

// NormalRampStrategy

Simulation::Scenario NormalRampStrategy::scenario() const
{
    return Simulation::Scenario::NormalRamp;
}

int NormalRampStrategy::startupSpeed(const Settings::Snapshot &) const
{
    return 700;
}

void NormalRampStrategy::reset() {}

void NormalRampStrategy::advance(TelemetryFrame &telemetry, const Settings::Snapshot &)
{
    telemetry.temperature += 1.2;
    telemetry.pressure += 1.5;
    telemetry.speed = std::min(3200, telemetry.speed + 100);
}

// OverloadStrategy

Simulation::Scenario OverloadStrategy::scenario() const
{
    return Simulation::Scenario::Overload;
}

int OverloadStrategy::startupSpeed(const Settings::Snapshot &) const
{
    return 900;
}

void OverloadStrategy::reset()
{
    m_ticks = 0;
}

void OverloadStrategy::advance(TelemetryFrame &telemetry, const Settings::Snapshot &settings)
{
    ++m_ticks;

    const double dt = secondsPerTick(settings);
    const double loadRamp = std::min(1.0, m_ticks / 12.0);
    const double speedRatio = telemetry.speed / 3600.0;
    const double heatSoak = std::max(0.0, (telemetry.temperature - 60.0) / 35.0);

    const int targetSpeed = static_cast<int>(3000 + (400 * loadRamp));
    const int speedStep = std::max(1, static_cast<int>((120 + (40 * loadRamp)) * dt));

    telemetry.speed = stepTowards(telemetry.speed, targetSpeed, speedStep);
    telemetry.temperature += (1.4 + (0.8 * loadRamp) + (0.5 * speedRatio)
                              + (0.5 * heatSoak))
                             * dt;
    telemetry.pressure += (0.9 + (0.7 * loadRamp) + (0.4 * speedRatio) + (0.3 * heatSoak))
                          * dt;
}

// CoolingFailureStrategy

Simulation::Scenario CoolingFailureStrategy::scenario() const
{
    return Simulation::Scenario::CoolingFailure;
}

int CoolingFailureStrategy::startupSpeed(const Settings::Snapshot &) const
{
    return 700;
}

void CoolingFailureStrategy::reset()
{
    m_ticks = 0;
    m_failureActive = false;
}

void CoolingFailureStrategy::advance(TelemetryFrame &telemetry, const Settings::Snapshot &)
{
    ++m_ticks;

    telemetry.temperature += 1.1;
    telemetry.pressure += 1.6;
    telemetry.speed = std::min(3200, telemetry.speed + 110);

    if (!m_failureActive && m_ticks >= 20) {
        m_failureActive = true;
    }

    if (m_failureActive) {
        telemetry.temperature += 2.8;
        telemetry.pressure += 0.6;
    }
}

// LoadStepResponseStrategy

Simulation::Scenario LoadStepResponseStrategy::scenario() const
{
    return Simulation::Scenario::LoadStepResponse;
}

int LoadStepResponseStrategy::startupSpeed(const Settings::Snapshot &) const
{
    return 700;
}

void LoadStepResponseStrategy::reset()
{
    m_ticks = 0;
    m_phase = Phase::Warmup;
}

void LoadStepResponseStrategy::advance(TelemetryFrame &telemetry, const Settings::Snapshot &settings)
{
    ++m_ticks;
    updatePhase();

    const double dt = secondsPerTick(settings);
    const double speedRatio = telemetry.speed / 3600.0;
    const double heatSoak = std::max(0.0, (telemetry.temperature - 65.0) / 30.0);
    const int speedStep = std::max(1, static_cast<int>(180 * dt));

    switch (m_phase) {
    case Phase::Warmup: {
        telemetry.temperature += 1.1 * dt;
        telemetry.pressure += 0.8 * dt;
        telemetry.speed = stepTowards(telemetry.speed, 2600, speedStep);
        break;
    }

    case Phase::LoadStep: {
        telemetry.temperature += (2.0 + (0.3 * speedRatio)) * dt;
        telemetry.pressure += (2.8 + (0.5 * speedRatio)) * dt;
        telemetry.speed = stepTowards(telemetry.speed, 2400, speedStep);
        break;
    }

    case Phase::ControlRecovery: {
        telemetry.temperature += (1.5 + (0.2 * heatSoak)) * dt;
        telemetry.pressure += 1.0 * dt;
        telemetry.speed = stepTowards(telemetry.speed, 2900, speedStep);
        break;
    }

    case Phase::LoadRelief: {
        telemetry.temperature += 0.5 * dt;
        telemetry.pressure += -0.3 * dt;
        telemetry.speed = stepTowards(telemetry.speed, 2700, speedStep);
        break;
    }
    }
}

void LoadStepResponseStrategy::updatePhase()
{
    if (m_ticks < 12) {
        m_phase = Phase::Warmup;
    } else if (m_ticks < 25) {
        m_phase = Phase::LoadStep;
    } else if (m_ticks < 40) {
        m_phase = Phase::ControlRecovery;
    } else {
        m_phase = Phase::LoadRelief;
    }
}
