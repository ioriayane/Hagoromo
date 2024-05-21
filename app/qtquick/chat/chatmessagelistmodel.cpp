#include "chatmessagelistmodel.h"

#include "atprotocol/lexicons_func_unknown.h"

using AtProtocolInterface::ChatBskyConvoGetMessages;
using namespace AtProtocolType::ChatBskyConvoDefs;
using namespace AtProtocolType;

ChatMessageListModel::ChatMessageListModel(QObject *parent)
    : AtpChatAbstractListModel { parent } { }

int ChatMessageListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_idList.count();
}

QVariant ChatMessageListModel::data(const QModelIndex &index, int role) const
{
    return item(index.row(), static_cast<ChatMessageListModelRoles>(role));
}

QVariant ChatMessageListModel::item(int row, ChatMessageListModelRoles role) const
{
    if (row < 0 || row >= m_idList.count())
        return QVariant();

    const auto &current = m_messageHash[m_idList.at(row)];

    if (role == IdRole)
        return current.id;
    else if (role == RevRole)
        return current.rev;
    else if (role == SenderDidRoles)
        return current.sender.did;
    else if (role == TextRoles)
        return current.text;
    else if (role == SentAtRoles)
        return LexiconsTypeUnknown::formatDateTime(current.sentAt);

    return QVariant();
}

bool ChatMessageListModel::getLatest()
{
    if (running() || convoId().isEmpty())
        return false;
    setRunning(true);

    getServiceEndpoint([=]() {
        ChatBskyConvoGetMessages *messages = new ChatBskyConvoGetMessages(this);
        connect(messages, &ChatBskyConvoGetMessages::finished, this, [=](bool success) {
            if (success) {
                if (m_idList.isEmpty() && m_cursor.isEmpty()) {
                    m_cursor = messages->cursor();
                }

                copyFrom(messages, true);
            } else {
                emit errorOccured(messages->errorCode(), messages->errorMessage());
            }
            setRunning(false);
            messages->deleteLater();
        });
        messages->setAccount(account());
        messages->setService(account().service_endpoint);
        messages->getMessages(convoId(), 0, QString());
    });

    return true;
}

bool ChatMessageListModel::getNext()
{
    if (running() || convoId().isEmpty() || m_cursor.isEmpty())
        return false;
    setRunning(true);

    getServiceEndpoint([=]() {
        ChatBskyConvoGetMessages *messages = new ChatBskyConvoGetMessages(this);
        connect(messages, &ChatBskyConvoGetMessages::finished, this, [=](bool success) {
            if (success) {
                m_cursor = messages->cursor();

                copyFrom(messages, false);
            } else {
                emit errorOccured(messages->errorCode(), messages->errorMessage());
            }
            setRunning(false);
            messages->deleteLater();
        });
        messages->setAccount(account());
        messages->setService(account().service_endpoint);
        messages->getMessages(convoId(), 0, m_cursor);
    });

    return true;
}

QHash<int, QByteArray> ChatMessageListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[IdRole] = "id";
    roles[RevRole] = "rev";

    roles[SenderDidRoles] = "senderDid";
    roles[TextRoles] = "text";
    roles[SentAtRoles] = "sentAt";

    return roles;
}

void ChatMessageListModel::copyFrom(const AtProtocolInterface::ChatBskyConvoGetMessages *messages,
                                    bool to_top)
{
    if (messages == nullptr)
        return;

    QStringList add_ids;
    for (auto item = messages->messagesMessageViewList().cbegin();
         item != messages->messagesMessageViewList().cend(); item++) {
        if (!m_idList.contains(item->id)) {
            if (to_top) {
                add_ids.insert(0, item->id);
            } else {
                add_ids.append(item->id);
            }
        }
        m_messageHash[item->id] = *item;
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

    for (auto item = messages->messagesDeletedMessageViewList().crbegin();
         item != messages->messagesDeletedMessageViewList().crend(); item++) {
        int row = m_idList.indexOf(item->id);
        if (row >= 0) {
            beginRemoveRows(QModelIndex(), row, row);
            m_idList.removeAt(row);
            endRemoveRows();
        }
    }
}

QString ChatMessageListModel::convoId() const
{
    return m_convoId;
}

void ChatMessageListModel::setConvoId(const QString &newConvoId)
{
    if (m_convoId == newConvoId)
        return;
    m_convoId = newConvoId;
    emit convoIdChanged();
}
