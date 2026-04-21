#include "log/log_interface.h"

#include "log/log_model.h"

LogInterface::LogInterface(LogModel &logModel, QObject *parent)
    : QObject(parent)
    , m_logModel(&logModel)
{
    Q_ASSERT(m_logModel);
}

void LogInterface::appendLog(const LogEvent &event)
{
    m_logModel->addLog(event);
}
