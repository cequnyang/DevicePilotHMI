#pragma once

#include <QPointer>
#include <QTimer>
#include <memory>

#include "backend/machine_backend.h"
#include "backend/simulation_scenario.h"
#include "backend/simulation_strategy.h"

class SettingsManager;

class SimulatedMachineBackend : public MachineBackend
{
    Q_OBJECT

public:
    explicit SimulatedMachineBackend(SettingsManager &settings, QObject *parent = nullptr);

    void requestStart() override;
    void requestStop() override;
    void requestResetFault() override;
    void requestSafeShutdown() override;

    Simulation::Scenario scenario() const;
    void setScenario(Simulation::Scenario scenario);

signals:
    void scenarioChanged();

private slots:
    void updateSimulation();
    void onTransitionTimeout();

private:
    enum class PendingTransition { None, FinishStart, FinishStop };

    void setState(MachineState newState);
    void publishTelemetry();
    void resetTelemetryToIdle();

private:
    QPointer<SettingsManager> m_settings{nullptr};
    MachineState m_state{MachineState::Idle};
    PendingTransition m_pendingTransition{PendingTransition::None};
    TelemetryFrame m_telemetry{};
    QTimer m_updateTimer;
    QTimer m_transitionTimer;
    Simulation::Scenario m_scenario{Simulation::Scenario::NormalRamp};
    std::unique_ptr<SimulationStrategy> m_simulationStrategy;
};
