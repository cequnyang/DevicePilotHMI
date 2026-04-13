#pragma once

#include <QObject>
#include <QPointer>
#include <QString>

class LogModel;

class LogInterface : public QObject
{
    Q_OBJECT

public:
    explicit LogInterface(LogModel &logModel, QObject *parent = nullptr);

    void appendLog(const QString &level, const QString &message);

private:
    QPointer<LogModel> m_logModel{nullptr};
};