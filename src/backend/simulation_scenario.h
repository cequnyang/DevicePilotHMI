#pragma once

#include <QMetaType>

namespace Simulation {
Q_NAMESPACE
enum class Scenario { NormalRamp, Overload, CoolingFailure, Unknown };

QString stateToString(Scenario state);
Scenario stringToState(QString string);

Q_ENUM_NS(Scenario)
} // namespace Simulation

Q_DECLARE_METATYPE(Simulation::Scenario)