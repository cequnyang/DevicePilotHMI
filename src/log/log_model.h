#pragma once

#include <QAbstractListModel>
#include <QString>
#include <QVector>
#include <qqmlintegration.h>

class LogModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("LogModel is created in C++ and exposed to QML as a model.")

public:
    explicit LogModel(QObject *parent = nullptr);

    enum Roles { TimestampRole = Qt::UserRole + 1, LevelRole, MessageRole, AcknowledgedRole };
    Q_ENUM(Roles)

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    Q_INVOKABLE void clear();

private:
    friend class LogInterface;
    void addLog(const QString &level, const QString &message);

private:
    struct Entry
    {
        QString timestamp;
        QString level;
        QString message;
        bool acknowledged{false};
    };

    QVector<Entry> m_entries;
    static constexpr int kMaxEntries = 200;
};