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

    Q_PROPERTY(bool visibleReplyToUnfollowedUsers READ visibleReplyToUnfollowedUsers WRITE
                       setVisibleReplyToUnfollowedUsers NOTIFY visibleReplyToUnfollowedUsersChanged)
    Q_PROPERTY(bool visibleRepostOfOwn READ visibleRepostOfOwn WRITE setVisibleRepostOfOwn NOTIFY
                       visibleRepostOfOwnChanged)
    Q_PROPERTY(bool visibleRepostOfFollowingUsers READ visibleRepostOfFollowingUsers WRITE
                       setVisibleRepostOfFollowingUsers NOTIFY visibleRepostOfFollowingUsersChanged)
    Q_PROPERTY(bool visibleRepostOfUnfollowingUsers READ visibleRepostOfUnfollowingUsers WRITE
                       setVisibleRepostOfUnfollowingUsers NOTIFY
                               visibleRepostOfUnfollowingUsersChanged)
    Q_PROPERTY(bool visibleRepostOfMine READ visibleRepostOfMine WRITE setVisibleRepostOfMine NOTIFY
                       visibleRepostOfMineChanged)
    Q_PROPERTY(bool visibleRepostByMe READ visibleRepostByMe WRITE setVisibleRepostByMe NOTIFY
                       visibleRepostByMeChanged)

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
        ReplyDisabledRole,
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

        HasListLinkRole,
        ListLinkUriRole,
        ListLinkCreatorHandleRole,
        ListLinkDisplayNameRole,
        ListLinkDescriptionRole,
        ListLinkAvatarRole,

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

        ThreadGateUriRole,
        ThreadGateTypeRole,
        ThreadGateRulesRole,

        LabelsRole,
        LanguagesRole,
        TagsRole,
        ViaRole,

        ThreadConnectedRole,
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
    virtual Q_INVOKABLE QString getOfficialUrl() const { return QString(); }
    virtual Q_INVOKABLE QString getItemOfficialUrl(int row) const;

    Q_INVOKABLE bool getLatest();
    Q_INVOKABLE bool getNext();
    Q_INVOKABLE bool deletePost(int row);
    Q_INVOKABLE bool repost(int row);
    Q_INVOKABLE bool like(int row);

    bool visibleReplyToUnfollowedUsers() const;
    void setVisibleReplyToUnfollowedUsers(bool newVisibleReplyToUnfollowedUser);
    bool visibleRepostOfOwn() const;
    void setVisibleRepostOfOwn(bool newVisibleRepostOfOwn);
    bool visibleRepostOfFollowingUsers() const;
    void setVisibleRepostOfFollowingUsers(bool newVisibleRepostOfFollowingUsers);
    bool visibleRepostOfUnfollowingUsers() const;
    void setVisibleRepostOfUnfollowingUsers(bool newVisibleRepostOfUnfollowingUsers);
    bool visibleRepostOfMine() const;
    void setVisibleRepostOfMine(bool newVisibleRepostOfMine);
    bool visibleRepostByMe() const;
    void setVisibleRepostByMe(bool newVisibleRepostByMe);

signals:
    void visibleReplyToUnfollowedUsersChanged();
    void visibleRepostOfOwnChanged();
    void visibleRepostOfFollowingUsersChanged();
    void visibleRepostOfUnfollowingUsersChanged();
    void visibleRepostOfMineChanged();
    void visibleRepostByMeChanged();

protected:
    QHash<int, QByteArray> roleNames() const;
    virtual void finishedDisplayingQueuedPosts();
    virtual bool checkVisibility(const QString &cid);
    void copyFrom(AtProtocolInterface::AppBskyFeedGetTimeline *timeline);
    void copyFromNext(AtProtocolInterface::AppBskyFeedGetTimeline *timeline);
    QString getReferenceTime(const AtProtocolType::AppBskyFeedDefs::FeedViewPost &view_post);
    QVariant getQuoteItem(const AtProtocolType::AppBskyFeedDefs::PostView &post,
                          const TimelineListModel::TimelineListModelRoles role) const;

    virtual void updateExtendMediaFile(const QString &parent_cid);

    QHash<QString, AtProtocolType::AppBskyFeedDefs::FeedViewPost> m_viewPostHash;
    QHash<QString, ThreadConnector> m_threadConnectorHash;

private:
    QHash<TimelineListModel::TimelineListModelRoles, AtpAbstractListModel::ExternalLinkRoles>
            m_toExternalLinkRoles;
    QHash<TimelineListModel::TimelineListModelRoles, AtpAbstractListModel::FeedGeneratorRoles>
            m_toFeedGeneratorRoles;
    QHash<TimelineListModel::TimelineListModelRoles, AtpAbstractListModel::ListLinkRoles>
            m_toListLinkRoles;
    QHash<TimelineListModel::TimelineListModelRoles, AtpAbstractListModel::ThreadGateRoles>
            m_toThreadGateRoles;

    bool m_visibleReplyToUnfollowedUsers;
    bool m_visibleRepostOfOwn;
    bool m_visibleRepostOfFollowingUsers;
    bool m_visibleRepostOfUnfollowingUsers;
    bool m_visibleRepostOfMine;
    bool m_visibleRepostByMe;
};

#endif // TIMELINELISTMODEL_H
