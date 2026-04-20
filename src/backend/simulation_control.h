#pragma once

#include <QObject>
#include <QPointer>
#include <QString>
#include <qqmlintegration.h>

#include "backend/simulation_scenario.h"

class SimulatedMachineBackend;

class SimulationControl : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("SimulationControl is created in C++ and injected into the root QML object.")

    Q_PROPERTY(Simulation::Scenario scenario READ scenario WRITE setScenario NOTIFY scenarioChanged)
    Q_PROPERTY(QString scenarioName READ scenarioName NOTIFY scenarioChanged)

public:
    explicit SimulationControl(SimulatedMachineBackend &backend, QObject *parent = nullptr);

    Simulation::Scenario scenario() const;
    void setScenario(Simulation::Scenario scenario);
    QString scenarioName() const;

signals:
    void scenarioChanged();

private:
    QPointer<SimulatedMachineBackend> m_backend{nullptr};
};
