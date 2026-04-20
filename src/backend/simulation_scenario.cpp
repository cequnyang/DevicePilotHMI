#include "backend/simulation_scenario.h"

QString Simulation::stateToString(Simulation::Scenario state)
{
    switch (state) {
    case Simulation::Scenario::NormalRamp:
        return "Normal Ramp";
        return "Warmup";
    case Simulation::Scenario::Overload:
        return "Overload";
    case Simulation::Scenario::CoolingFailure:
        return "Cooling Failure";
    case Simulation::Scenario::LoadStepResponse:
        return "Load Step Response";
    default:
        break;
    }
    return "Unknown";
}