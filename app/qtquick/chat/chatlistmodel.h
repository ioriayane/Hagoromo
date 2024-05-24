#ifndef CHATLISTMODEL_H
#define CHATLISTMODEL_H

#include "atpchatabstractlistmodel.h"
#include "atprotocol/chat/bsky/convo/chatbskyconvolistconvos.h"

class ChatListModel : public AtpChatAbstractListModel
{
    Q_OBJECT
public:
    explicit ChatListModel(QObject *parent = nullptr);

    // モデルで提供する項目のルールID的な（QML側へ公開するために大文字で始めること）
    enum ChatListModelRoles {
        ModelData = Qt::UserRole + 1,
        IdRole,
        RevRole,

        MemberHandlesRole,
        MemberDisplayNamesRole,
        MemberAvatarsRole,

        LastMessageIdRole,
        LastMessageRevRole,
        LastMessageSenderDidRole,
        LastMessageTextRole,
        LastMessageSentAtRole,

        UnreadCountRole,
    };
    Q_ENUM(ChatListModelRoles);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    Q_INVOKABLE QVariant item(int row, ChatListModel::ChatListModelRoles role) const;
    Q_INVOKABLE void update(int row, ChatListModel::ChatListModelRoles role, const QVariant &value);

    virtual Q_INVOKABLE bool getLatest();
    virtual Q_INVOKABLE bool getNext();

protected:
    QHash<int, QByteArray> roleNames() const;

    void copyFrom(const AtProtocolInterface::ChatBskyConvoListConvos *convos, bool to_top);

private:
    QHash<QString, AtProtocolType::ChatBskyConvoDefs::ConvoView> m_convoHash; // QHash<id, convo>
};

#endif // CHATLISTMODEL_H
