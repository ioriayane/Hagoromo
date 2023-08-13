#ifndef NOTIFICATIONLISTMODEL_H
#define NOTIFICATIONLISTMODEL_H

#include "atprotocol/lexicons.h"
#include "atpabstractlistmodel.h"

#include <QAbstractListModel>
#include <QObject>

class NotificationListModel : public AtpAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)

    Q_PROPERTY(bool visibleLike READ visibleLike WRITE setVisibleLike NOTIFY visibleLikeChanged)
    Q_PROPERTY(bool visibleRepost READ visibleRepost WRITE setVisibleRepost NOTIFY
                       visibleRepostChanged)
    Q_PROPERTY(bool visibleFollow READ visibleFollow WRITE setVisibleFollow NOTIFY
                       visibleFollowChanged)
    Q_PROPERTY(bool visibleMention READ visibleMention WRITE setVisibleMention NOTIFY
                       visibleMentionChanged)
    Q_PROPERTY(bool visibleReply READ visibleReply WRITE setVisibleReply NOTIFY visibleReplyChanged)
    Q_PROPERTY(bool visibleQuote READ visibleQuote WRITE setVisibleQuote NOTIFY visibleQuoteChanged)
public:
    explicit NotificationListModel(QObject *parent = nullptr);

    // モデルで提供する項目のルールID的な（QML側へ公開するために大文字で始めること）
    enum NotificationListModelRoles {
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
        // EmbedImagesRole,
        // EmbedImagesFullRole,
        IsRepostedRole,
        IsLikedRole,
        RepostedUriRole,
        LikedUriRole,

        ReasonRole,

        RecordCidRole,
        RecordUriRole,
        RecordDisplayNameRole,
        RecordHandleRole,
        RecordAvatarRole,
        RecordIndexedAtRole,
        RecordRecordTextRole,
        RecordImagesRole,
        RecordImagesFullRole,
        RecordIsRepostedRole,
        RecordIsLikedRole,

        HasGeneratorFeedRole,
        GeneratorFeedUriRole,
        GeneratorFeedCreatorHandleRole,
        GeneratorFeedDisplayNameRole,
        GeneratorFeedLikeCountRole,
        GeneratorFeedAvatarRole,

        UserFilterMatchedRole,
        UserFilterMessageRole,
    };
    Q_ENUM(NotificationListModelRoles)

    enum NotificationListModelReason {
        ReasonUnknown,
        ReasonLike,
        ReasonRepost,
        ReasonFollow,
        ReasonMention,
        ReasonReply,
        ReasonQuote,
    };
    Q_ENUM(NotificationListModelReason)

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    Q_INVOKABLE QVariant item(int row,
                              NotificationListModel::NotificationListModelRoles role) const;
    Q_INVOKABLE void update(int row, NotificationListModel::NotificationListModelRoles role,
                            const QVariant &value);
    Q_INVOKABLE void clear();

    virtual Q_INVOKABLE int indexOf(const QString &cid) const;
    virtual Q_INVOKABLE QString getRecordText(const QString &cid);

    Q_INVOKABLE void getLatest();
    Q_INVOKABLE void repost(int row);
    Q_INVOKABLE void like(int row);

    bool visibleLike() const;
    void setVisibleLike(bool newVisibleLike);
    bool visibleRepost() const;
    void setVisibleRepost(bool newVisibleRepost);
    bool visibleFollow() const;
    void setVisibleFollow(bool newVisibleFollow);
    bool visibleMention() const;
    void setVisibleMention(bool newVisibleMention);
    bool visibleReply() const;
    void setVisibleReply(bool newVisibleReply);
    bool visibleQuote() const;
    void setVisibleQuote(bool newVisibleQuote);

signals:
    void visibleLikeChanged();
    void visibleRepostChanged();
    void visibleFollowChanged();
    void visibleMentionChanged();
    void visibleReplyChanged();
    void visibleQuoteChanged();

protected:
    QHash<int, QByteArray> roleNames() const;
    virtual void finishedDisplayingQueuedPosts();
    virtual bool checkVisibility(const QString &cid);

private:
    //    QList<QString> m_cidList; //
    //    これで取得したポストの順番を管理して実態はm_notificationHashで管理
    QHash<QString, AtProtocolType::AppBskyNotificationListNotifications::Notification>
            m_notificationHash;

    QHash<QString, AtProtocolType::AppBskyFeedDefs::PostView> m_postHash;
    QStringList m_cueGetPost;

    void getPosts();

    template<typename T>
    void appendGetPostCue(const QVariant &record);
    template<typename T>
    void emitRecordDataChanged(const int i, const QStringList &new_cid, const QVariant &record);

    bool enableReason(const QString &reason) const;

    bool m_visibleLike;
    bool m_visibleRepost;
    bool m_visibleFollow;
    bool m_visibleMention;
    bool m_visibleReply;
    bool m_visibleQuote;
};

#endif // NOTIFICATIONLISTMODEL_H
