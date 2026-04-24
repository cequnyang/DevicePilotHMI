#pragma once

#include <QLoggingCategory>
#include <QString>

struct LogEvent;

Q_DECLARE_LOGGING_CATEGORY(lcApp)
Q_DECLARE_LOGGING_CATEGORY(lcBootstrap)
Q_DECLARE_LOGGING_CATEGORY(lcRuntime)
Q_DECLARE_LOGGING_CATEGORY(lcAlarm)
Q_DECLARE_LOGGING_CATEGORY(lcSettings)
Q_DECLARE_LOGGING_CATEGORY(lcPersistence)
Q_DECLARE_LOGGING_CATEGORY(lcBackend)
Q_DECLARE_LOGGING_CATEGORY(lcUi)

namespace AppLogging {
void initialize();
void shutdown();

bool fileLoggingEnabled();
QString sessionLogFilePath();
QString lastError();

void mirrorEvent(const LogEvent &event);
} // namespace AppLogging
