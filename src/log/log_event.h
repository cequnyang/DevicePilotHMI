#pragma once

#include <QString>

struct LogEvent
{
    QString level;
    QString source;
    QString eventType;
    QString message;
};
