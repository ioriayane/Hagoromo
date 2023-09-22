#ifndef TIMELINELISTMODEL_H
#define TIMELINELISTMODEL_H

#include "atprotocol/lexicons.h"
#include "atprotocol/app/bsky/feed/appbskyfeedgettimeline.h"
#include "atpabstractlistmodel.h"

#include <QAbstractListModel>
#include <QObject>

struct ThreadConnector
{
    bool top = false;
    bool bottom = false;
};

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
        MutedRole,
        RecordTextRole,
        RecordTextPlainRole,
        RecordTextTranslationRole,
        ReplyCountRole,
        RepostCountRole,
        LikeCountRole,
        IndexedAtRole,
        IndexedAtLongRole,
        EmbedImagesRole,
        EmbedImagesFullRole,
        EmbedImagesAltRole,

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
        QuoteRecordEmbedImagesAltRole,
        QuoteRecordBlockedRole,

        HasExternalLinkRole,
        ExternalLinkUriRole,
        ExternalLinkTitleRole,
        ExternalLinkDescriptionRole,
        ExternalLinkThumbRole,

        HasFeedGeneratorRole,
        FeedGeneratorUriRole,
        FeedGeneratorCreatorHandleRole,
        FeedGeneratorDisplayNameRole,
        FeedGeneratorLikeCountRole,
        FeedGeneratorAvatarRole,

        HasReplyRole,
        ReplyRootCidRole,
        ReplyRootUriRole,
        ReplyParentDisplayNameRole,
        ReplyParentHandleRole,
        IsRepostedByRole,
        RepostedByDisplayNameRole,
        RepostedByHandleRole,

        UserFilterMatchedRole,
        UserFilterMessageRole,
        ContentFilterMatchedRole,
        ContentFilterMessageRole,
        ContentMediaFilterMatchedRole,
        ContentMediaFilterMessageRole,
        QuoteFilterMatchedRole,

        LabelsRole,
        LanguagesRole,
        ViaRole,

        ThreadConnectorTopRole,
        ThreadConnectorBottomRole,
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
    Q_INVOKABLE void getNext();
    Q_INVOKABLE void deletePost(int row);
    Q_INVOKABLE void repost(int row);
    Q_INVOKABLE void like(int row);

protected:
    QHash<int, QByteArray> roleNames() const;
    virtual void finishedDisplayingQueuedPosts();
    virtual bool checkVisibility(const QString &cid);
    void copyFrom(AtProtocolInterface::AppBskyFeedGetTimeline *timeline);
    void copyFromNext(AtProtocolInterface::AppBskyFeedGetTimeline *timeline);
    QString getReferenceTime(const AtProtocolType::AppBskyFeedDefs::FeedViewPost &view_post);
    QVariant getQuoteItem(const AtProtocolType::AppBskyFeedDefs::PostView &post,
                          const TimelineListModel::TimelineListModelRoles role) const;

    QHash<QString, AtProtocolType::AppBskyFeedDefs::FeedViewPost> m_viewPostHash;
    QHash<QString, ThreadConnector> m_threadConnectorHash;
};

#endif // TIMELINELISTMODEL_H
