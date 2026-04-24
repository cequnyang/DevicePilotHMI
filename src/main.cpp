#include <QCoreApplication>
#include <QGuiApplication>
#include <QIcon>
#include <QQuickStyle>
#include <QQmlApplicationEngine>
#include <QVariant>
#include <QWindow>
#include <qqml.h>

#include "alarm/alarm_manager.h"
#include "backend/simulated_machine_backend.h"
#include "log/app_logging.h"
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

    QQuickStyle::setStyle("Basic");
    QGuiApplication app(argc, argv);
    AppLogging::initialize();

    if (AppLogging::fileLoggingEnabled()) {
        qCInfo(lcBootstrap).noquote()
            << QString("Session log file: %1").arg(AppLogging::sessionLogFilePath());
    } else {
        const QString reason = AppLogging::lastError().isEmpty()
                                   ? QString("unknown initialization failure")
                                   : AppLogging::lastError();
        qCWarning(lcBootstrap).noquote() << QString("File logging disabled (%1)").arg(reason);
    }
    qCInfo(lcBootstrap) << "Application startup";

    LogModel logModel;
    LogInterface logInterface(logModel);
    SettingsManager settingsManager(logInterface);
    SimulatedMachineBackend simulatedBackend(logInterface, settingsManager);
    SimulationControl simulationControl(simulatedBackend);
    MachineRuntime machineRuntime(logInterface, simulatedBackend);
    AlarmManager alarmManager(logInterface, settingsManager, machineRuntime);
    SettingsApplyService settingsApplyService(logInterface, settingsManager, machineRuntime);
    SettingsSession settingsSession(logInterface, settingsManager, settingsApplyService);

    const auto updateWindowIcon = [&app, &alarmManager]() {
        QString iconPath = QStringLiteral(":/icons/app_icon_normal.png");
        if (alarmManager.isFault()) {
            iconPath = QStringLiteral(":/icons/app_icon_fault.png");
        } else if (alarmManager.hasWarning()) {
            iconPath = QStringLiteral(":/icons/app_icon_warning.png");
        }
        const QIcon icon(iconPath);
        app.setWindowIcon(icon);
        for (QWindow *window : app.topLevelWindows()) {
            if (window != nullptr) {
                window->setIcon(icon);
            }
        }
    };
    updateWindowIcon();
    QObject::connect(&alarmManager, &AlarmManager::alarmChanged, &app, updateWindowIcon);

    QQmlApplicationEngine engine;
    LogFilterProxyModel filteredLogModel;
    filteredLogModel.setSourceLogModel(&logModel);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() {
            qCCritical(lcBootstrap) << "QML object creation failed. Exiting.";
            QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    engine.setInitialProperties({{"simulCtrl", QVariant::fromValue(&simulationControl)},
                                 {"runtime", QVariant::fromValue(&machineRuntime)},
                                 {"alarm", QVariant::fromValue(&alarmManager)},
                                 {"logModel", QVariant::fromValue(&logModel)},
                                 {"filteredLogModel", QVariant::fromValue(&filteredLogModel)},
                                 {"settingsManager", QVariant::fromValue(&settingsManager)},
                                 {"settingsSession", QVariant::fromValue(&settingsSession)}});

    qCInfo(lcBootstrap) << "Loading main QML module";
    engine.loadFromModule("DevicePilotHMI", "Main");
    if (!engine.rootObjects().isEmpty()) {
        qCInfo(lcBootstrap) << "Main QML module loaded";
    }

    const int exitCode = app.exec();
    qCInfo(lcBootstrap) << "Application exiting with code" << exitCode;
    AppLogging::shutdown();
    return exitCode;
}
