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

    Q_PROPERTY(bool enabledLike READ enabledLike WRITE setEnabledLike NOTIFY enabledLikeChanged)
    Q_PROPERTY(bool enabledRepost READ enabledRepost WRITE setEnabledRepost NOTIFY
                       enabledRepostChanged)
    Q_PROPERTY(bool enabledFollow READ enabledFollow WRITE setEnabledFollow NOTIFY
                       enabledFollowChanged)
    Q_PROPERTY(bool enabledMention READ enabledMention WRITE setEnabledMention NOTIFY
                       enabledMentionChanged)
    Q_PROPERTY(bool enabledReply READ enabledReply WRITE setEnabledReply NOTIFY enabledReplyChanged)
    Q_PROPERTY(bool enabledQuote READ enabledQuote WRITE setEnabledQuote NOTIFY enabledQuoteChanged)
public:
    explicit NotificationListModel(QObject *parent = nullptr);

    // モデルで提供する項目のルールID的な（QML側へ公開するために大文字で始めること）
    enum NotificationListModelRoles {
        ModelData = Qt::UserRole + 1,
        VisibleRole,
        CidRole,
        UriRole,
        DidRole,
        DisplayNameRole,
        HandleRole,
        AvatarRole,
        RecordTextRole,
        RecordTextPlainRole,
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

    virtual Q_INVOKABLE int indexOf(const QString &cid) const;
    virtual Q_INVOKABLE QString getRecordText(const QString &cid);

    Q_INVOKABLE void getLatest();
    Q_INVOKABLE void repost(int row);
    Q_INVOKABLE void like(int row);

    bool enabledLike() const;
    void setEnabledLike(bool newEnabledLike);
    bool enabledRepost() const;
    void setEnabledRepost(bool newEnabledRepost);
    bool enabledFollow() const;
    void setEnabledFollow(bool newEnabledFollow);
    bool enabledMention() const;
    void setEnabledMention(bool newEnabledMention);
    bool enabledReply() const;
    void setEnabledReply(bool newEnabledReply);
    bool enabledQuote() const;
    void setEnabledQuote(bool newEnabledQuote);

signals:
    void enabledLikeChanged();
    void enabledRepostChanged();
    void enabledFollowChanged();
    void enabledMentionChanged();
    void enabledReplyChanged();
    void enabledQuoteChanged();

protected:
    QHash<int, QByteArray> roleNames() const;
    virtual void finishedDisplayingQueuedPosts();

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

    bool m_enabledLike;
    bool m_enabledRepost;
    bool m_enabledFollow;
    bool m_enabledMention;
    bool m_enabledReply;
    bool m_enabledQuote;
};

#endif // NOTIFICATIONLISTMODEL_H
