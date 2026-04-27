#include "log/app_logging.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>
#include <QStandardPaths>

#include <algorithm>

#include "log/log_event.h"

Q_LOGGING_CATEGORY(lcApp, "devicepilothmi.app", QtInfoMsg)
Q_LOGGING_CATEGORY(lcBootstrap, "devicepilothmi.bootstrap", QtInfoMsg)
Q_LOGGING_CATEGORY(lcRuntime, "devicepilothmi.runtime", QtInfoMsg)
Q_LOGGING_CATEGORY(lcAlarm, "devicepilothmi.alarm", QtInfoMsg)
Q_LOGGING_CATEGORY(lcSettings, "devicepilothmi.settings", QtInfoMsg)
Q_LOGGING_CATEGORY(lcPersistence, "devicepilothmi.persistence", QtInfoMsg)
Q_LOGGING_CATEGORY(lcBackend, "devicepilothmi.backend", QtInfoMsg)
Q_LOGGING_CATEGORY(lcUi, "devicepilothmi.ui", QtInfoMsg)

namespace {
constexpr int kMaxRetainedSessionFiles = 10;

QString logFilePrefix()
{
    QString appName = QCoreApplication::applicationName().trimmed().toLower();
    if (appName.isEmpty()) {
        appName = "devicepilothmi";
    }

    appName.replace(' ', '-');
    return appName;
}

QString resolveLogDirectoryPath()
{
    QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    if (basePath.isEmpty()) {
        basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    }
    if (basePath.isEmpty()) {
        basePath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    }
    if (basePath.isEmpty()) {
        return {};
    }

    return QDir(basePath).filePath("logs");
}

QString buildSessionLogFilePath(const QString &directoryPath)
{
    const QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss-zzz");
    return QDir(directoryPath)
        .filePath(QString("%1-%2-%3.log")
                      .arg(logFilePrefix(), timestamp)
                      .arg(QCoreApplication::applicationPid()));
}

const QLoggingCategory &categoryForEventSource(const QString &source)
{
    if (source == "alarm") {
        return lcAlarm();
    }
    if (source == "runtime") {
        return lcRuntime();
    }
    if (source == "settings") {
        return lcSettings();
    }
    if (source == "persistence") {
        return lcPersistence();
    }
    if (source == "backend") {
        return lcBackend();
    }
    if (source == "ui" || source == "qml") {
        return lcUi();
    }

    return lcApp();
}

bool isCategoryEnabled(const QLoggingCategory &category, QtMsgType type)
{
    switch (type) {
    case QtDebugMsg:
        return category.isDebugEnabled();
    case QtInfoMsg:
        return category.isInfoEnabled();
    case QtWarningMsg:
        return category.isWarningEnabled();
    case QtCriticalMsg:
    case QtFatalMsg:
        return category.isCriticalEnabled();
    }

    return true;
}

QtMsgType messageTypeForEvent(const LogEvent &event)
{
    if (event.level == "FAULT") {
        return QtCriticalMsg;
    }

    if (event.level == "WARNING") {
        return QtWarningMsg;
    }

    if (event.eventType.endsWith(".failed") || event.eventType.endsWith(".rejected")
        || event.eventType.contains(".repaired")) {
        return QtWarningMsg;
    }

    return QtInfoMsg;
}

QString formatEventMessage(const LogEvent &event)
{
    if (event.message.isEmpty()) {
        return event.eventType;
    }

    return QString("%1 | %2").arg(event.eventType, event.message);
}

void writeCategorizedMessage(const QLoggingCategory &category,
                             QtMsgType type,
                             const QString &message)
{
    if (!isCategoryEnabled(category, type)) {
        return;
    }

    const QMessageLogger logger(__FILE__, __LINE__, Q_FUNC_INFO, category.categoryName());

    switch (type) {
    case QtDebugMsg:
        logger.debug().noquote() << message;
        return;
    case QtInfoMsg:
        logger.info().noquote() << message;
        return;
    case QtWarningMsg:
        logger.warning().noquote() << message;
        return;
    case QtCriticalMsg:
        logger.critical().noquote() << message;
        return;
    case QtFatalMsg:
        logger.fatal("%s", qPrintable(message));
        return;
    }
}

class SessionFileLogger
{
public:
    static SessionFileLogger &instance()
    {
        static SessionFileLogger logger;
        return logger;
    }

    void initialize()
    {
        const const QMutexLocker locker(&m_mutex);
        if (m_initialized) {
            return;
        }

        m_initialized = true;
        m_lastError.clear();
        m_logDirectoryPath = resolveLogDirectoryPath();
        if (m_logDirectoryPath.isEmpty()) {
            m_lastError = "No writable log directory is available.";
            m_previousHandler = qInstallMessageHandler(&SessionFileLogger::messageHandler);
            return;
        }

        const QDir directory;
        if (!directory.mkpath(m_logDirectoryPath)) {
            m_lastError = QString("Failed to create log directory (%1)").arg(m_logDirectoryPath);
            m_previousHandler = qInstallMessageHandler(&SessionFileLogger::messageHandler);
            return;
        }

        cleanupOldSessionLogsLocked();

        m_logFilePath = buildSessionLogFilePath(m_logDirectoryPath);
        m_file.setFileName(m_logFilePath);
        if (!m_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            m_lastError = QString("Failed to open session log file (%1)").arg(m_file.errorString());
            m_logFilePath.clear();
        }

        m_previousHandler = qInstallMessageHandler(&SessionFileLogger::messageHandler);
        if (m_file.isOpen()) {
            writeLineLocked(QString("===== session started %1 =====")
                                .arg(QDateTime::currentDateTime().toString(Qt::ISODateWithMs)));
        }
    }

    void shutdown()
    {
        const QMutexLocker locker(&m_mutex);
        if (!m_initialized) {
            return;
        }

        if (m_file.isOpen()) {
            writeLineLocked(QString("===== session ended %1 =====")
                                .arg(QDateTime::currentDateTime().toString(Qt::ISODateWithMs)));
            m_file.flush();
            m_file.close();
        }

        qInstallMessageHandler(m_previousHandler);
        m_previousHandler = nullptr;
        m_initialized = false;
    }

    bool fileLoggingEnabled() const
    {
        const QMutexLocker locker(&m_mutex);
        return m_file.isOpen();
    }

    QString sessionLogFilePath() const
    {
        const QMutexLocker locker(&m_mutex);
        return m_logFilePath;
    }

    QString lastError() const
    {
        const QMutexLocker locker(&m_mutex);
        return m_lastError;
    }

    void handleMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
    {
        const QString formatted = qFormatLogMessage(type, context, msg);
        {
            const QMutexLocker locker(&m_mutex);
            if (m_file.isOpen()) {
                writeLineLocked(formatted);
            }
        }

        if (m_previousHandler != nullptr) {
            m_previousHandler(type, context, msg);
        }
    }

private:
    static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
    {
        instance().handleMessage(type, context, msg);
    }

    void cleanupOldSessionLogsLocked()
    {
        const QDir directory(m_logDirectoryPath);
        QFileInfoList files = directory.entryInfoList({QString("%1-*.log").arg(logFilePrefix())},
                                                      QDir::Files);
        std::sort(files.begin(), files.end(), [](const QFileInfo &left, const QFileInfo &right) {
            return left.lastModified() > right.lastModified();
        });

        for (int index = kMaxRetainedSessionFiles - 1; index < files.size(); ++index) {
            QFile::remove(files.at(index).absoluteFilePath());
        }
    }

    void writeLineLocked(const QString &line)
    {
        if (!m_file.isOpen()) {
            return;
        }

        m_file.write(line.toUtf8());
        m_file.write("\n");
        m_file.flush();
    }

private:
    mutable QMutex m_mutex;
    QFile m_file;
    QString m_logDirectoryPath;
    QString m_logFilePath;
    QString m_lastError;
    QtMessageHandler m_previousHandler{nullptr};
    bool m_initialized{false};
};
} // namespace

void AppLogging::initialize()
{
    qSetMessagePattern(
        "[%{time yyyy-MM-dd hh:mm:ss.zzz}] [%{type}] [%{category}] [pid:%{pid}] [tid:%{threadid}] %{message}");
    SessionFileLogger::instance().initialize();
}

void AppLogging::shutdown()
{
    SessionFileLogger::instance().shutdown();
}

bool AppLogging::fileLoggingEnabled()
{
    return SessionFileLogger::instance().fileLoggingEnabled();
}

QString AppLogging::sessionLogFilePath()
{
    return SessionFileLogger::instance().sessionLogFilePath();
}

QString AppLogging::lastError()
{
    return SessionFileLogger::instance().lastError();
}

void AppLogging::mirrorEvent(const LogEvent &event)
{
    writeCategorizedMessage(categoryForEventSource(event.source),
                            messageTypeForEvent(event),
                            formatEventMessage(event));
}
