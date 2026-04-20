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
    case Simulation::Scenario::Unknown:
        break;
    }

    return std::make_unique<NormalRampStrategy>();
}

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
}; // namespace

void OverloadStrategy::advance(TelemetryFrame &telemetry, const Settings::Snapshot &settings)
{
    ++m_ticks;

    const double dt = secondsPerTick(settings);
    const double loadRamp = std::min(1.0, m_ticks / 12.0);
    const double speedRatio = telemetry.speed / 3600.0;
    const double heatSoak = std::max(0.0, (telemetry.temperature - 60.0) / 35.0);

    const int targetSpeed = static_cast<int>(3000 + 400 * loadRamp);
    const int speedStep = std::max(1, static_cast<int>((120 + 40 * loadRamp) * dt));

    telemetry.speed = stepTowards(telemetry.speed, targetSpeed, speedStep);
    telemetry.temperature += (1.4 + 0.8 * loadRamp + 0.5 * speedRatio + 0.5 * heatSoak) * dt;
    telemetry.pressure += (0.9 + 0.7 * loadRamp + 0.4 * speedRatio + 0.3 * heatSoak) * dt;
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