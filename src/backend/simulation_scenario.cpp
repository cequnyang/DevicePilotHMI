#include "backend/simulation_scenario.h"

QString Simulation::stateToString(Simulation::Scenario state)
{
    switch (state) {
    case Simulation::Scenario::NormalRamp:
        return "NormalRamp";
    case Simulation::Scenario::Overload:
        return "Overload";
    case Simulation::Scenario::CoolingFailure:
        return "CoolingFailure";
    default:
        break;
    }
    return "Unknown";
}

Simulation::Scenario Simulation::stringToState(QString string)
{
    if (string == "NormalRamp") {
        return Scenario::NormalRamp;
    }
    if (string == "Overload") {
        return Scenario::Overload;
    }
    if (string == "CoolingFailure") {
        return Scenario::CoolingFailure;
    }
    return Simulation::Scenario::Unknown;
}
