#pragma once

#include <memory>

#include "backend/simulation_scenario.h"
#include "runtime/machine_types.h"
#include "settings/settings_defined.h"

class SimulationStrategy
{
public:
    virtual ~SimulationStrategy() = default;

    virtual Simulation::Scenario scenario() const = 0;

    virtual int startupSpeed(const Settings::Snapshot &settings) const = 0;
    virtual void reset() = 0;
    virtual void advance(TelemetryFrame &telemetry, const Settings::Snapshot &settings) = 0;
};

std::unique_ptr<SimulationStrategy> makeSimulationStrategy(Simulation::Scenario scenario);

class NormalRampStrategy final : public SimulationStrategy
{
public:
    Simulation::Scenario scenario() const override;
    int startupSpeed(const Settings::Snapshot &) const override;
    void reset() override;
    void advance(TelemetryFrame &telemetry, const Settings::Snapshot &) override;
};

class OverloadStrategy final : public SimulationStrategy
{
public:
    Simulation::Scenario scenario() const override;
    int startupSpeed(const Settings::Snapshot &) const override;
    void reset() override;
    void advance(TelemetryFrame &telemetry, const Settings::Snapshot &settings) override;

private:
    double secondsPerTick(const Settings::Snapshot &settings);
    int stepTowards(int current, int target, int step);

private:
    int m_ticks{0};
};

class CoolingFailureStrategy final : public SimulationStrategy
{
public:
    Simulation::Scenario scenario() const override;
    int startupSpeed(const Settings::Snapshot &) const override;
    void reset() override;
    void advance(TelemetryFrame &telemetry, const Settings::Snapshot &) override;

private:
    int m_ticks{0};
    bool m_failureActive{false};
};
