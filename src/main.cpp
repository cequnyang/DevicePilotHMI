#include <QCoreApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QVariant>
#include <qqml.h>

#include "alarm/alarm_manager.h"
#include "backend/simulated_machine_backend.h"
#include "backend/simulation_control.h"
#include "log/log_filter_proxy_model.h"
#include "log/log_interface.h"
#include "log/log_model.h"
#include "runtime/machine_runtime.h"
#include "settings/settings_apply_service.h"
#include "settings/settings_manager.h"
#include "settings/settings_session.h"

int main(int argc, char *argv[])
{
    qRegisterMetaType<MachineState>();
    qRegisterMetaType<TelemetryFrame>();
    qRegisterMetaType<Simulation::Scenario>();

    qmlRegisterUncreatableMetaObject(Simulation::staticMetaObject,
                                     "DevicePilotHMI",
                                     1,
                                     0,
                                     "SimulationScenario",
                                     "Enums only");

    QGuiApplication app(argc, argv);

    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    SimulatedMachineBackend simulatedBackend(settingsManager);
    SimulationControl simulationControl(simulatedBackend);
    MachineRuntime machineRuntime(logInterface, simulatedBackend);
    AlarmManager alarmManager(logInterface, settingsManager, machineRuntime);
    SettingsApplyService settingsApplyService(logInterface, settingsManager, machineRuntime);
    SettingsSession settingsSession(logInterface, settingsManager, settingsApplyService);

    QQmlApplicationEngine engine;
    LogFilterProxyModel filteredLogModel;
    filteredLogModel.setSourceLogModel(&logModel);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.setInitialProperties({{"simulCtrl", QVariant::fromValue(&simulationControl)},
                                 {"runtime", QVariant::fromValue(&machineRuntime)},
                                 {"alarm", QVariant::fromValue(&alarmManager)},
                                 {"logModel", QVariant::fromValue(&logModel)},
                                 {"filteredLogModel", QVariant::fromValue(&filteredLogModel)},
                                 {"settingsSession", QVariant::fromValue(&settingsSession)}});

    engine.loadFromModule("DevicePilotHMI", "Main");

    return app.exec();
}
