#pragma once

#include <QObject>
#include <QPointer>

#include "log/log_event.h"

class LogModel;

class LogInterface : public QObject
{
    Q_OBJECT

public:
    explicit LogInterface(LogModel &logModel, QObject *parent = nullptr);

    void appendLog(const LogEvent &event);

private:
    QPointer<LogModel> m_logModel{nullptr};
};
