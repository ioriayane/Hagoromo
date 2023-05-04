#ifndef TIMELINELISTMODEL_H
#define TIMELINELISTMODEL_H

#include "../atprotocol/lexicons.h"
#include "../atprotocol/appbskyfeedgettimeline.h"
#include "accountlistmodel.h"

#include <QAbstractListModel>
#include <QObject>

class TimelineListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit TimelineListModel(QObject *parent = nullptr);

    // モデルで提供する項目のルールID的な（QML側へ公開するために大文字で始めること）
    enum TimelineListModelRoles {
        ModelData = Qt::UserRole + 1,
        DisplayNameRole,
        HandleRole,
        AvatarRole,
        RecordTextRole,
        ReplyCountRole,
        RepostCountRole,
        LikeCountRole,
        IndexedAtRole,
        EmbedImagesRole,

        HasParentRole,
        ParentDisPlayName,
    };
    Q_ENUM(TimelineListModelRoles)

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    Q_INVOKABLE QVariant item(int row, TimelineListModel::TimelineListModelRoles role) const;
    Q_INVOKABLE void update(int row, TimelineListModel::TimelineListModelRoles role,
                            const QVariant &value);
    Q_INVOKABLE void updateAccount(const QString &service, const QString &did,
                                   const QString &handle, const QString &email,
                                   const QString &accessJwt, const QString &refreshJwt);

    Q_INVOKABLE void getLatest();

signals:
    void serviceChanged();

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    QList<QString> m_cidList; // これで取得したポストの順番を管理して実態はm_viewPostListで管理
    QHash<QString, AppBskyFeedDefs::FeedViewPost> m_viewPostHash;
    AppBskyFeedGetTimeline m_timeline;

    AccountItem m_account;
};

#endif // TIMELINELISTMODEL_H
