#include "backend/simulation_control.h"

#include "backend/simulated_machine_backend.h"

SimulationControl::SimulationControl(SimulatedMachineBackend &backend, QObject *parent)
    : QObject(parent)
    , m_backend(&backend)
{
    Q_ASSERT(m_backend);

    connect(m_backend, &SimulatedMachineBackend::scenarioChanged, this, &SimulationControl::scenarioChanged);
}

Simulation::Scenario SimulationControl::scenario() const
{
    return m_backend->scenario();
}

void SimulationControl::setScenario(Simulation::Scenario scenario)
{
    m_backend->setScenario(scenario);
}

QString SimulationControl::scenarioName() const
{
    return Simulation::stateToString(m_backend->scenario());
}
