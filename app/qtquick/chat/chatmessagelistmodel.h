#ifndef CHATMESSAGELISTMODEL_H
#define CHATMESSAGELISTMODEL_H

#include "atpchatabstractlistmodel.h"
#include "atprotocol/chat/bsky/convo/chatbskyconvogetlog.h"
#include "tools/chatlogsubscriber.h"
#include "tools/configurablelabels.h"

class ChatMessageListModel : public AtpChatAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QString convoId READ convoId WRITE setConvoId NOTIFY convoIdChanged FINAL)
    Q_PROPERTY(QStringList memberDids READ memberDids WRITE setMemberDids NOTIFY memberDidsChanged
                       FINAL)
    Q_PROPERTY(bool runSending READ runSending WRITE setRunSending NOTIFY runSendingChanged FINAL)
    Q_PROPERTY(bool ready READ ready NOTIFY readyChanged FINAL)

    Q_PROPERTY(QStringList memberAvatars READ memberAvatars WRITE setMemberAvatars NOTIFY
                       memberAvatarsChanged FINAL)
    Q_PROPERTY(QStringList memberHandles READ memberHandles WRITE setMemberHandles NOTIFY
                       memberHandlesChanged FINAL)
    Q_PROPERTY(QStringList memberDisplayNames READ memberDisplayNames WRITE setMemberDisplayNames
                       NOTIFY memberDisplayNamesChanged FINAL)
public:
    explicit ChatMessageListModel(QObject *parent = nullptr);
    ~ChatMessageListModel();

    // モデルで提供する項目のルールID的な（QML側へ公開するために大文字で始めること）
    enum ChatMessageListModelRoles {
        ModelData = Qt::UserRole + 1,
        IdRole,
        RevRole,

        SenderDidRole,
        SenderAvatarRole,

        TextRole,
        TextPlainRole,
        SentAtRole,
        ReactionEmojisRole,

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

        QuoteFilterMatchedRole,

        RunningRole,
    };
    Q_ENUM(ChatMessageListModelRoles);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    Q_INVOKABLE QVariant item(int row, ChatMessageListModel::ChatMessageListModelRoles role) const;
    Q_INVOKABLE void update(int row, ChatMessageListModel::ChatMessageListModelRoles role,
                            const QVariant &value);

    virtual Q_INVOKABLE bool getLatest();
    virtual Q_INVOKABLE bool getNext();

    Q_INVOKABLE void send(const QString &message, const QString &embed_uri,
                          const QString &embed_cid);
    Q_INVOKABLE void deleteMessage(int row);

    QString convoId() const;
    void setConvoId(const QString &newConvoId);
    bool runSending() const;
    void setRunSending(bool newRunSending);
    QStringList memberDids() const;
    void setMemberDids(const QStringList &newMemberDids);
    bool ready() const;
    void setReady(bool newReady);
    QStringList memberAvatars() const;
    void setMemberAvatars(const QStringList &newMemberAvatars);
    QStringList memberHandles() const;
    void setMemberHandles(const QStringList &newMemberHandles);
    QStringList memberDisplayNames() const;
    void setMemberDisplayNames(const QStringList &newMemberDisplayNames);

signals:
    void finishSent(bool success);
    void errorOccured(const QString &code, const QString &message);
    void convoIdChanged();
    void runSendingChanged();
    void memberDidsChanged();
    void readyChanged();
    void memberAvatarsChanged();
    void memberHandlesChanged();
    void memberDisplayNamesChanged();

public slots:
    void receiveLogs(const AtProtocolInterface::ChatBskyConvoGetLog &log);
    void errorLogs(const QString &code, const QString &message);

protected:
    QHash<int, QByteArray> roleNames() const;

    void
    copyFrom(const QList<AtProtocolType::ChatBskyConvoDefs::MessageView> &messages,
             const QList<AtProtocolType::ChatBskyConvoDefs::DeletedMessageView> &deletedMessages,
             const QList<AtProtocolType::ChatBskyConvoDefs::MessageView> &removedReactions,
             bool to_top);

private:
    void getConvo(const QString &convoId, const QStringList &memberDids,
                  std::function<void()> callback);
    void getLogCursor(std::function<void()> callback);
    void setMembers(const AtProtocolType::ChatBskyConvoDefs::ConvoView &convo);

    QHash<QString, AtProtocolType::ChatBskyConvoDefs::MessageView> m_messageHash;
    AtProtocolType::ChatBskyConvoDefs::ConvoView m_convo;
    ChatLogConnector m_chatLogConnector;
    QString m_logCursor;
    QString m_convoId;
    bool m_runSending;
    QStringList m_memberDids;
    bool m_ready;
    QStringList m_memberAvatars;
    QStringList m_memberHandles;
    QStringList m_memberDisplayNames;
};

#endif // CHATMESSAGELISTMODEL_H
