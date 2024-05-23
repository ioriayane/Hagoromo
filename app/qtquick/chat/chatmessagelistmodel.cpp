#include "chatmessagelistmodel.h"

#include "atprotocol/chat/bsky/convo/chatbskyconvogetconvo.h"
#include "atprotocol/chat/bsky/convo/chatbskyconvogetlog.h"
#include "atprotocol/chat/bsky/convo/chatbskyconvogetmessages.h"
#include "atprotocol/chat/bsky/convo/chatbskyconvosendmessage.h"
#include "atprotocol/lexicons_func_unknown.h"

using AtProtocolInterface::ChatBskyConvoGetConvo;
using AtProtocolInterface::ChatBskyConvoGetLog;
using AtProtocolInterface::ChatBskyConvoGetMessages;
using AtProtocolInterface::ChatBskyConvoSendMessage;
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
    else if (role == SenderAvatarRoles) {
        for (const auto &member : m_convo.members) {
            if (member.did == current.sender.did) {
                return member.avatar;
            }
        }
        return QString();
    } else if (role == TextRoles)
        return LexiconsTypeUnknown::applyFacetsTo(current.text, current.facets);
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
        getConvo(convoId(), [=]() {
            QString cursor;
            if (!m_idList.isEmpty()) {
                const auto &latest = m_messageHash[m_idList.first()];
                if (!latest.rev.isEmpty()) {
                    cursor = latest.rev;
                }
            }
            if (cursor.isEmpty()) {
                ChatBskyConvoGetMessages *messages = new ChatBskyConvoGetMessages(this);
                connect(messages, &ChatBskyConvoGetMessages::finished, this, [=](bool success) {
                    if (success) {
                        if (m_idList.isEmpty() && m_cursor.isEmpty()) {
                            m_cursor = messages->cursor();
                        }
                        copyFrom(messages->messagesMessageViewList(),
                                 messages->messagesDeletedMessageViewList(), true);
                    } else {
                        emit errorOccured(messages->errorCode(), messages->errorMessage());
                    }
                    setRunning(false);
                    messages->deleteLater();
                });
                messages->setAccount(account());
                messages->setService(account().service_endpoint);
                messages->getMessages(convoId(), 0, QString());
            } else {
                ChatBskyConvoGetLog *log = new ChatBskyConvoGetLog(this);
                connect(log, &ChatBskyConvoGetLog::finished, this, [=](bool success) {
                    if (success) {
                        QList<AtProtocolType::ChatBskyConvoDefs::MessageView> messages;
                        for (const auto msg : log->logsLogCreateMessageList()) {
                            if (msg.message_type
                                == LogCreateMessageMessageType::message_MessageView) {
                                messages.insert(0, msg.message_MessageView);
                            }
                        }
                        copyFrom(messages,
                                 QList<AtProtocolType::ChatBskyConvoDefs::DeletedMessageView>(),
                                 true);

                    } else {
                        emit errorOccured(log->errorCode(), log->errorMessage());
                    }
                    setRunning(false);
                    log->deleteLater();
                });
                log->setAccount(account());
                log->setService(account().service_endpoint);
                log->getLog(cursor);
            }
        });
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

                copyFrom(messages->messagesMessageViewList(),
                         messages->messagesDeletedMessageViewList(), false);
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

void ChatMessageListModel::send(const QString &message)
{
    if (running() || convoId().isEmpty())
        return;
    setRunning(true);

    LexiconsTypeUnknown::makeFacets(
            this, account(), message,
            [=](const QList<AtProtocolType::AppBskyRichtextFacet::Main> &facets) {
                QJsonObject obj;
                obj.insert("text", message);
                LexiconsTypeUnknown::insertFacetsJson(obj, facets);

                ChatBskyConvoSendMessage *convo = new ChatBskyConvoSendMessage(this);
                connect(convo, &ChatBskyConvoSendMessage::finished, this, [=](bool success) {
                    if (success) {
                        qDebug() << "Sent" << convo->messageView().id << convo->messageView().text;
                        // QList<AtProtocolType::ChatBskyConvoDefs::MessageView> messages;
                        // messages.append(convo->messageView());
                        // copyFrom(messages,
                        //          QList<AtProtocolType::ChatBskyConvoDefs::DeletedMessageView>(),
                        //          true);
                    } else {
                        emit errorOccured(convo->errorCode(), convo->errorMessage());
                    }
                    setRunning(false);
                    convo->deleteLater();
                });
                convo->setAccount(account());
                convo->setService(account().service_endpoint);
                convo->sendMessage(convoId(), obj);
            });
}

QHash<int, QByteArray> ChatMessageListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[IdRole] = "id";
    roles[RevRole] = "rev";

    roles[SenderDidRoles] = "senderDid";
    roles[SenderAvatarRoles] = "senderAvatar";

    roles[TextRoles] = "text";
    roles[SentAtRoles] = "sentAt";

    return roles;
}

void ChatMessageListModel::copyFrom(const QList<MessageView> &messages,
                                    const QList<DeletedMessageView> &deletedMessages, bool to_top)
{
    QStringList add_ids;
    for (auto item = messages.cbegin(); item != messages.cend(); item++) {
        m_messageHash[item->id] = *item;
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

    for (auto item = deletedMessages.crbegin(); item != deletedMessages.crend(); item++) {
        int row = m_idList.indexOf(item->id);
        if (row >= 0) {
            beginRemoveRows(QModelIndex(), row, row);
            m_idList.removeAt(row);
            endRemoveRows();
        }
    }
}

void ChatMessageListModel::getConvo(const QString &convoId, std::function<void()> callback)
{
    if (convoId.isEmpty() || !m_convo.id.isEmpty()) {
        callback();
        return;
    }

    ChatBskyConvoGetConvo *convo = new ChatBskyConvoGetConvo(this);
    connect(convo, &ChatBskyConvoGetConvo::finished, [=](bool success) {
        if (success) {
            m_convo = convo->convo();
        } else {
            m_convo.id.clear();
        }
        callback();
        convo->deleteLater();
    });
    convo->setAccount(account());
    convo->setService(account().service_endpoint);
    convo->getConvo(convoId);
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
