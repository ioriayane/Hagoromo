#include "chatlistmodel.h"
#include "atprotocol/lexicons_func_unknown.h"
#include "atprotocol/chat/bsky/convo/chatbskyconvomuteconvo.h"
#include "atprotocol/chat/bsky/convo/chatbskyconvounmuteconvo.h"
#include "atprotocol/chat/bsky/convo/chatbskyconvoleaveconvo.h"

using AtProtocolInterface::ChatBskyConvoLeaveConvo;
using AtProtocolInterface::ChatBskyConvoListConvos;
using AtProtocolInterface::ChatBskyConvoMuteConvo;
using AtProtocolInterface::ChatBskyConvoUnmuteConvo;
using namespace AtProtocolType::ChatBskyConvoDefs;
using namespace AtProtocolType;

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

    else if (role == MemberHandlesRole) {
        QStringList handles;
        for (const auto &member : current.members) {
            if (member.did != account().did) {
                handles.append(member.handle);
            }
        }
        return handles;
    } else if (role == MemberDisplayNamesRole) {
        QStringList names;
        for (const auto &member : current.members) {
            if (member.did != account().did) {
                names.append(member.displayName);
            }
        }
        return names;
    } else if (role == MemberAvatarsRole) {
        QStringList avatars;
        for (const auto &member : current.members) {
            if (member.did != account().did) {
                avatars.append(member.avatar);
            }
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
    else if (role == LastMessageSentAtRole)
        return current.lastMessage_type == ConvoViewLastMessageType::lastMessage_MessageView
                ? LexiconsTypeUnknown::formatDateTime(current.lastMessage_MessageView.sentAt, true)
                : QString();

    else if (role == MutedRole)
        return current.muted;
    else if (role == UnreadCountRole)
        return current.unreadCount;
    else if (role == RunningRole) {
        return m_itemRunningHash.value(m_idList.at(row), false);
    }

    return QVariant();
}

void ChatListModel::update(int row, ChatListModelRoles role, const QVariant &value)
{
    if (row < 0 || row >= m_idList.count())
        return;

    auto &current = m_convoHash[m_idList.at(row)];

    if (role == MutedRole) {
        int muted = value.toBool();
        if (current.muted != muted) {
            current.muted = muted;
            emit dataChanged(index(row), index(row));
        }
    } else if (role == UnreadCountRole) {
        int count = value.toInt();
        if (current.unreadCount != count) {
            qDebug() << "update value" << current.unreadCount << "->" << count;
            current.unreadCount = value.toInt();
            emit dataChanged(index(row), index(row));
        }
    } else if (role == RunningRole) {
        bool running = m_itemRunningHash.value(m_idList.at(row), false);
        if (running != value.toBool()) {
            m_itemRunningHash[m_idList.at(row)] = value.toBool();
            emit dataChanged(index(row), index(row));
        }
    }
}

bool ChatListModel::getLatest()
{
    if (running())
        return false;
    setRunning(true);

    getServiceEndpoint([=]() {
        updateContentFilterLabels([=]() {
            ChatBskyConvoListConvos *convos = new ChatBskyConvoListConvos(this);
            connect(convos, &ChatBskyConvoListConvos::finished, this, [=](bool success) {
                if (success) {
                    if (m_idList.isEmpty() && m_cursor.isEmpty()) {
                        m_cursor = convos->cursor();
                    }

                    copyFrom(convos, true);
                } else {
                    emit errorOccured(convos->errorCode(), convos->errorMessage());
                    checkScopeError(convos->errorCode(), convos->errorMessage());
                }
                setRunning(false);
                convos->deleteLater();
            });
            convos->setAccount(account());
            convos->setService(account().service_endpoint);
            convos->setLabelers(labelerDids());
            convos->listConvos(0, QString(), QString(), QString());
        });
    });

    return true;
}

bool ChatListModel::getNext()
{
    if (running() || m_cursor.isEmpty())
        return false;
    setRunning(true);

    getServiceEndpoint([=]() {
        ChatBskyConvoListConvos *convos = new ChatBskyConvoListConvos(this);
        connect(convos, &ChatBskyConvoListConvos::finished, this, [=](bool success) {
            if (success) {
                m_cursor = convos->cursor();

                copyFrom(convos, false);
            } else {
                emit errorOccured(convos->errorCode(), convos->errorMessage());
            }
            setRunning(false);
            convos->deleteLater();
        });
        convos->setAccount(account());
        convos->setService(account().service_endpoint);
        convos->listConvos(0, m_cursor, QString(), QString());
    });

    return true;
}

void ChatListModel::mute(int row)
{
    if (row < 0 || row >= m_idList.count())
        return;
    const auto &current = m_convoHash.value(m_idList.at(row));
    bool running = item(row, RunningRole).toBool();
    if (running)
        return;
    update(row, RunningRole, true);
    if (current.muted) {
        // -> unmute
        ChatBskyConvoUnmuteConvo *convo = new ChatBskyConvoUnmuteConvo(this);
        connect(convo, &ChatBskyConvoUnmuteConvo::finished, this, [=](bool success) {
            if (success) {
                update(row, MutedRole, false);
            } else {
                emit errorOccured(convo->errorCode(), convo->errorMessage());
            }
            update(row, RunningRole, false);
            convo->deleteLater();
        });
        convo->setAccount(account());
        convo->setService(account().service_endpoint);
        convo->unmuteConvo(current.id);
    } else {
        // -> mute
        ChatBskyConvoMuteConvo *convo = new ChatBskyConvoMuteConvo(this);
        connect(convo, &ChatBskyConvoMuteConvo::finished, this, [=](bool success) {
            if (success) {
                update(row, MutedRole, true);
            } else {
                emit errorOccured(convo->errorCode(), convo->errorMessage());
            }
            update(row, RunningRole, false);
            convo->deleteLater();
        });
        convo->setAccount(account());
        convo->setService(account().service_endpoint);
        convo->muteConvo(current.id);
    }
}

void ChatListModel::leave(int row)
{
    if (row < 0 || row >= m_idList.count())
        return;
    const auto &current = m_convoHash.value(m_idList.at(row));
    bool running = item(row, RunningRole).toBool();
    if (running)
        return;
    update(row, RunningRole, true);

    // ChatBskyConvoLeaveConvo
    ChatBskyConvoLeaveConvo *convo = new ChatBskyConvoLeaveConvo(this);
    connect(convo, &ChatBskyConvoLeaveConvo::finished, this, [=](bool success) {
        update(row, RunningRole, false);
        if (success) {
            beginRemoveRows(QModelIndex(), row, row);
            m_idList.removeAt(row);
            endRemoveRows();
        } else {
            emit errorOccured(convo->errorCode(), convo->errorMessage());
        }
        convo->deleteLater();
    });
    convo->setAccount(account());
    convo->setService(account().service_endpoint);
    convo->leaveConvo(current.id);
}

QHash<int, QByteArray> ChatListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[IdRole] = "id";
    roles[RevRole] = "rev";

    roles[MemberHandlesRole] = "memberHandles";
    roles[MemberDisplayNamesRole] = "memberDisplayNames";
    roles[MemberAvatarsRole] = "memberAvatars";

    roles[LastMessageIdRole] = "lastMessageId";
    roles[LastMessageRevRole] = "lastMessageRev";
    roles[LastMessageSenderDidRole] = "lastMessageSenderDid";
    roles[LastMessageTextRole] = "lastMessageText";
    roles[LastMessageSentAtRole] = "lastMessageSentAt";

    roles[MutedRole] = "muted";
    roles[UnreadCountRole] = "unreadCountRole";
    roles[RunningRole] = "running";

    return roles;
}

void ChatListModel::copyFrom(const AtProtocolInterface::ChatBskyConvoListConvos *convos,
                             bool to_top)
{
    if (convos == nullptr)
        return;

    QStringList add_ids;

    for (auto item = convos->convosList().cbegin(); item != convos->convosList().cend(); item++) {
        m_convoHash[item->id] = *item;
        if (!m_idList.contains(item->id)) {
            if (to_top) {
                add_ids.insert(0, item->id);
            } else {
                add_ids.append(item->id);
            }
        } else {
            int row = m_idList.indexOf(item->id);
            emit dataChanged(index(row), index(row));
        }
    }

    if (!add_ids.isEmpty()) {
        int basis_pos = 0;
        if (!to_top) {
            basis_pos = rowCount();
        }
        beginInsertRows(QModelIndex(), basis_pos, basis_pos + add_ids.count() - 1);
        for (const auto &id : add_ids) {
            if (to_top) {
                m_idList.insert(0, id);
            } else {
                m_idList.append(id);
            }
        }
        endInsertRows();
    }
}
