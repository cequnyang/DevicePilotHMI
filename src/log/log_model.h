#pragma once

#include <QAbstractListModel>
#include <QString>
#include <QVector>
#include <qqmlintegration.h>

#include "log/log_event.h"

class LogModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("LogModel is created in C++ and exposed to QML as a model.")

public:
    explicit LogModel(QObject *parent = nullptr);

    enum Roles {
        TimestampRole = Qt::UserRole + 1,
        LevelRole,
        SourceRole,
        EventTypeRole,
        MessageRole,
        AcknowledgedRole
    };
    Q_ENUM(Roles)

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    Q_INVOKABLE void clear();

private:
    friend class LogInterface;
    void addLog(const LogEvent &event);

private:
    struct Entry
    {
        QString timestamp;
        QString level;
        QString source;
        QString eventType;
        QString message;
        bool acknowledged{false};
    };

    QVector<Entry> m_entries;
    static constexpr int kMaxEntries = 200;
};