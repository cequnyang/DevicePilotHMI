#pragma once

#include <QSortFilterProxyModel>
#include <qqmlintegration.h>

#include "log/log_model.h"

class LogFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(LogModel *sourceLogModel READ sourceLogModel WRITE setSourceLogModel NOTIFY
                   sourceLogModelChanged)
    Q_PROPERTY(QString levelFilter READ levelFilter WRITE setLevelFilter NOTIFY levelFilterChanged)
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)
    Q_PROPERTY(bool showOnlyUnacknowledged READ showOnlyUnacknowledged WRITE
                   setShowOnlyUnacknowledged NOTIFY showOnlyUnacknowledgedChanged)

public:
    explicit LogFilterProxyModel(QObject *parent = nullptr);

    LogModel *sourceLogModel() const;
    void setSourceLogModel(LogModel *model);

    QString levelFilter() const;
    void setLevelFilter(const QString &level);

    QString searchText() const;
    void setSearchText(const QString &text);

    bool showOnlyUnacknowledged() const;
    void setShowOnlyUnacknowledged(bool value);

    Q_INVOKABLE void setAcknowledged(int proxyRow, bool acknowledged);

signals:
    void sourceLogModelChanged();
    void levelFilterChanged();
    void searchTextChanged();
    void showOnlyUnacknowledgedChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    LogModel *m_sourceLogModel{nullptr};
    QString m_levelFilter;
    QString m_searchText;
    bool m_showOnlyUnacknowledged{false};
};
