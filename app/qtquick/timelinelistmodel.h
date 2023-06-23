#ifndef TIMELINELISTMODEL_H
#define TIMELINELISTMODEL_H

#include "atprotocol/lexicons.h"
#include "atprotocol/app/bsky/feed/appbskyfeedgettimeline.h"
#include "atpabstractlistmodel.h"

#include <QAbstractListModel>
#include <QObject>

class TimelineListModel : public AtpAbstractListModel
{
    Q_OBJECT

public:
    explicit TimelineListModel(QObject *parent = nullptr);

    // モデルで提供する項目のルールID的な（QML側へ公開するために大文字で始めること）
    enum TimelineListModelRoles {
        ModelData = Qt::UserRole + 1,
        CidRole,
        UriRole,
        DidRole,
        DisplayNameRole,
        HandleRole,
        AvatarRole,
        RecordTextRole,
        RecordTextPlainRole,
        RecordTextTranslationRole,
        ReplyCountRole,
        RepostCountRole,
        LikeCountRole,
        IndexedAtRole,
        EmbedImagesRole,
        EmbedImagesFullRole,

        IsRepostedRole,
        IsLikedRole,
        RepostedUriRole,
        LikedUriRole,

        HasQuoteRecordRole,
        QuoteRecordCidRole,
        QuoteRecordUriRole,
        QuoteRecordDisplayNameRole,
        QuoteRecordHandleRole,
        QuoteRecordAvatarRole,
        QuoteRecordRecordTextRole,
        QuoteRecordIndexedAtRole,
        QuoteRecordEmbedImagesRole,
        QuoteRecordEmbedImagesFullRole,

        HasExternalLinkRole,
        ExternalLinkUriRole,
        ExternalLinkTitleRole,
        ExternalLinkDescriptionRole,
        ExternalLinkThumbRole,

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
    virtual Q_INVOKABLE int indexOf(const QString &cid) const;
    virtual Q_INVOKABLE QString getRecordText(const QString &cid);

    Q_INVOKABLE void getLatest();
    Q_INVOKABLE void repost(int row);
    Q_INVOKABLE void like(int row);

protected:
    QHash<int, QByteArray> roleNames() const;
    virtual void finishedDisplayingQueuedPosts();
    void copyFrom(AtProtocolInterface::AppBskyFeedGetTimeline *timeline);
    QString getReferenceTime(const AtProtocolType::AppBskyFeedDefs::FeedViewPost &view_post);

    QHash<QString, AtProtocolType::AppBskyFeedDefs::FeedViewPost> m_viewPostHash;
};

#endif // TIMELINELISTMODEL_H
