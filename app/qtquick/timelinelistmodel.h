#ifndef TIMELINELISTMODEL_H
#define TIMELINELISTMODEL_H

#include "atprotocol/lexicons.h"
#include "atprotocol/appbskyfeedgettimeline.h"
#include "atpabstractlistmodel.h"

#include <QAbstractListModel>
#include <QObject>

class TimelineListModel : public AtpAbstractListModel
{
    Q_OBJECT

public:
    explicit TimelineListModel(AtpAbstractListModel *parent = nullptr);

    // モデルで提供する項目のルールID的な（QML側へ公開するために大文字で始めること）
    enum TimelineListModelRoles {
        ModelData = Qt::UserRole + 1,
        CidRole,
        UriRole,
        DisplayNameRole,
        HandleRole,
        AvatarRole,
        RecordTextRole,
        ReplyCountRole,
        RepostCountRole,
        LikeCountRole,
        IndexedAtRole,
        EmbedImagesRole,
        EmbedImagesFullRole,

        HasQuoteRecordRole,
        QuoteRecordDisplayNameRole,
        QuoteRecordHandleRole,
        QuoteRecordAvatarRole,
        QuoteRecordRecordTextRole,
        QuoteRecordIndexedAtRole,
        QuoteRecordEmbedImagesRole,
        QuoteRecordEmbedImagesFullRole,

        HasReplyRole,
        ReplyRootCidRole,
        ReplyRootUriRole,
        ReplyParentDisplayNameRole,
        ReplyParentHandleRole,
        IsRepostedByRole,
        RepostedByDisplayNameRole,
        RepostedByHandleRole,
    };
    Q_ENUM(TimelineListModelRoles)

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    Q_INVOKABLE QVariant item(int row, TimelineListModel::TimelineListModelRoles role) const;
    Q_INVOKABLE void update(int row, TimelineListModel::TimelineListModelRoles role,
                            const QVariant &value);

    Q_INVOKABLE void getLatest();

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    QList<QString> m_cidList; // これで取得したポストの順番を管理して実態はm_viewPostHashで管理
    QHash<QString, AtProtocolType::AppBskyFeedDefs::FeedViewPost> m_viewPostHash;
    AtProtocolInterface::AppBskyFeedGetTimeline m_timeline;
};

#endif // TIMELINELISTMODEL_H
