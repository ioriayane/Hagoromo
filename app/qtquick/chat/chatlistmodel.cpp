#include "chatlistmodel.h"

#include "atprotocol/chat/bsky/convo/chatbskyconvolistconvos.h"

using AtProtocolInterface::ChatBskyConvoListConvos;
using namespace AtProtocolType::ChatBskyConvoDefs;

ChatListModel::ChatListModel(QObject *parent) : AtpChatAbstractListModel { parent } { }

int ChatListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_idList.count();
}

QVariant ChatListModel::data(const QModelIndex &index, int role) const
{
    return item(index.row(), static_cast<ChatListModelRoles>(role));
}

QVariant ChatListModel::item(int row, ChatListModelRoles role) const
{
    if (row < 0 || row >= m_idList.count())
        return QVariant();

    const auto &current = m_convoHash.value(m_idList.at(row));

    if (role == IdRole)
        return current.id;
    else if (role == RevRole)
        return current.rev;

    else if (role == MemberDisplayNamesRole) {
        QStringList names;
        for (const auto &member : current.members) {
            names.append(member.displayName);
        }
        return names;
    } else if (role == MemberAvatarsRole) {
        QStringList avatars;
        for (const auto &member : current.members) {
            avatars.append(member.avatar);
        }
        return avatars;

    } else if (role == LastMessageIdRole)
        return current.lastMessage_type == ConvoViewLastMessageType::lastMessage_MessageView
                ? current.lastMessage_MessageView.id
                : QString();
    else if (role == LastMessageRevRole)
        return current.lastMessage_type == ConvoViewLastMessageType::lastMessage_MessageView
                ? current.lastMessage_MessageView.rev
                : QString();
    else if (role == LastMessageSenderDidRole)
        return current.lastMessage_type == ConvoViewLastMessageType::lastMessage_MessageView
                ? current.lastMessage_MessageView.sender.did
                : QString();
    else if (role == LastMessageTextRole)
        return current.lastMessage_type == ConvoViewLastMessageType::lastMessage_MessageView
                ? current.lastMessage_MessageView.text
                : QString();
    else if (role == LastMessageUnreadCountRole)
        return current.unreadCount;

    return QVariant();
}

bool ChatListModel::getLatest()
{
    if (running())
        return false;
    setRunning(true);

    ChatBskyConvoListConvos *convos = new ChatBskyConvoListConvos(this);
    connect(convos, &ChatBskyConvoListConvos::finished, this, [=](bool success) {
        if (success) {
            if (m_idList.isEmpty() && m_cursor.isEmpty()) {
                m_cursor = convos->cursor();
            }

            beginInsertRows(QModelIndex(), 0, convos->convosList().count() - 1);
            for (auto item = convos->convosList().crbegin(); item != convos->convosList().crend();
                 item++) {
                m_convoHash[item->id] = *item;
                m_idList.insert(0, item->id);
            }
            endInsertRows();
        } else {
            emit errorOccured(convos->errorCode(), convos->errorMessage());
        }
        setRunning(false);
        convos->deleteLater();
    });
    convos->setAccount(account());
    convos->appendRawHeader(headerName, headerValue);
    convos->listConvos(0, QString());
    return true;
}

bool ChatListModel::getNext()
{
    if (running())
        return false;
    setRunning(true);

    ChatBskyConvoListConvos *convos = new ChatBskyConvoListConvos(this);
    connect(convos, &ChatBskyConvoListConvos::finished, this, [=](bool success) {
        if (success) {
            m_cursor = convos->cursor();

            beginInsertRows(QModelIndex(), m_idList.count(),
                            m_idList.count() + convos->convosList().count() - 1);
            for (auto item = convos->convosList().cbegin(); item != convos->convosList().cend();
                 item++) {
                m_convoHash[item->id] = *item;
                m_idList.append(item->id);
            }
            endInsertRows();
        } else {
            emit errorOccured(convos->errorCode(), convos->errorMessage());
        }
        setRunning(false);
        convos->deleteLater();
    });
    convos->setAccount(account());
    convos->appendRawHeader(headerName, headerValue);
    convos->listConvos(0, m_cursor);

    return true;
}

QHash<int, QByteArray> ChatListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[IdRole] = "id";
    roles[RevRole] = "rev";

    roles[MemberDisplayNamesRole] = "memberDisplayNames";
    roles[MemberAvatarsRole] = "memberAvatars";

    roles[LastMessageIdRole] = "lastMessageId";
    roles[LastMessageRevRole] = "lastMessageRev";
    roles[LastMessageSenderDidRole] = "lastMessageSenderDid";
    roles[LastMessageTextRole] = "lastMessageTextRole";
    roles[LastMessageUnreadCountRole] = "lastMessageUnreadCountRole";

    return roles;
}
