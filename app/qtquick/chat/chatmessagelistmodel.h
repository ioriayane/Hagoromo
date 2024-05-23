#ifndef CHATMESSAGELISTMODEL_H
#define CHATMESSAGELISTMODEL_H

#include "atpchatabstractlistmodel.h"

class ChatMessageListModel : public AtpChatAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QString convoId READ convoId WRITE setConvoId NOTIFY convoIdChanged FINAL)
    Q_PROPERTY(bool runSending READ runSending WRITE setRunSending NOTIFY runSendingChanged FINAL)

public:
    explicit ChatMessageListModel(QObject *parent = nullptr);
    ~ChatMessageListModel();

    // モデルで提供する項目のルールID的な（QML側へ公開するために大文字で始めること）
    enum ChatMessageListModelRoles {
        ModelData = Qt::UserRole + 1,
        IdRole,
        RevRole,

        SenderDidRoles,
        SenderAvatarRoles,

        TextRoles,
        SentAtRoles,
    };
    Q_ENUM(ChatMessageListModelRoles);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    Q_INVOKABLE QVariant item(int row, ChatMessageListModel::ChatMessageListModelRoles role) const;

    virtual Q_INVOKABLE bool getLatest();
    virtual Q_INVOKABLE bool getNext();

    Q_INVOKABLE void send(const QString &message);

    QString convoId() const;
    void setConvoId(const QString &newConvoId);
    bool runSending() const;
    void setRunSending(bool newRunSending);

signals:
    void finishSent(bool success);
    void convoIdChanged();

    void runSendingChanged();

protected:
    QHash<int, QByteArray> roleNames() const;

    void
    copyFrom(const QList<AtProtocolType::ChatBskyConvoDefs::MessageView> &messages,
             const QList<AtProtocolType::ChatBskyConvoDefs::DeletedMessageView> &deletedMessages,
             bool to_top);

private:
    void getConvo(const QString &convoId, std::function<void()> callback);

    QHash<QString, AtProtocolType::ChatBskyConvoDefs::MessageView> m_messageHash;
    AtProtocolType::ChatBskyConvoDefs::ConvoView m_convo;
    QString m_convoId;
    bool m_runSending;
};

#endif // CHATMESSAGELISTMODEL_H
