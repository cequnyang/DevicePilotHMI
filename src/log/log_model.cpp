#include "log_model.h"

#include <QDateTime>

LogModel::LogModel(QObject *parent)
    : QAbstractListModel(parent)
{}

int LogModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return static_cast<int>(m_entries.size());
}

QVariant LogModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    const int row = index.row();
    if (row < 0 || m_entries.size() <= row) {
        return {};
    }

    const Entry &entry = m_entries[row];

    switch (role) {
    case TimestampRole:
        return entry.timestamp;
    case LevelRole:
        return entry.level;
    case MessageRole:
        return entry.message;
    case AcknowledgedRole:
        return entry.acknowledged;
    default:
        return {};
    }
}

QHash<int, QByteArray> LogModel::roleNames() const
{
    return {{TimestampRole, "timestamp"},
            {LevelRole, "level"},
            {MessageRole, "message"},
            {AcknowledgedRole, "acknowledged"}};
}

Qt::ItemFlags LogModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

bool LogModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) {
        return false;
    }

    const int row = index.row();
    if (row < 0 || m_entries.size() <= row) {
        return false;
    }

    Entry &entry = m_entries[row];

    switch (role) {
    case AcknowledgedRole: {
        const bool acknowledged = value.toBool();
        if (entry.acknowledged == acknowledged) {
            return false;
        }

        entry.acknowledged = acknowledged;
        emit dataChanged(index, index, {AcknowledgedRole});
        return true;
    }
    default:
        return false;
    }
}

void LogModel::addLog(const QString &level, const QString &message)
{
    if (m_entries.size() >= kMaxEntries) {
        beginRemoveRows(QModelIndex(), 0, 0);
        m_entries.removeFirst();
        endRemoveRows();
    }

    const int newRow = static_cast<int>(m_entries.size());

    beginInsertRows(QModelIndex(), newRow, newRow);
    m_entries.emplace_back(Entry{.timestamp = QDateTime::currentDateTime().toString("HH:mm:ss"),
                                 .level = level,
                                 .message = message});
    endInsertRows();
}

void LogModel::clear()
{
    if (m_entries.isEmpty()) {
        return;
    }

    beginResetModel();
    m_entries.clear();
    endResetModel();
}