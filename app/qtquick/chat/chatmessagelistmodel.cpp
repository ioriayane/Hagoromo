#include "chatmessagelistmodel.h"

#include "atprotocol/chat/bsky/convo/chatbskyconvodeletemessageforself.h"
#include "atprotocol/chat/bsky/convo/chatbskyconvogetconvo.h"
#include "atprotocol/chat/bsky/convo/chatbskyconvogetconvoformembers.h"
#include "atprotocol/chat/bsky/convo/chatbskyconvogetlog.h"
#include "atprotocol/chat/bsky/convo/chatbskyconvogetmessages.h"
#include "atprotocol/chat/bsky/convo/chatbskyconvolistconvos.h"
#include "atprotocol/chat/bsky/convo/chatbskyconvosendmessage.h"
#include "atprotocol/lexicons_func_unknown.h"
#include "tools/chatlogsubscriber.h"

using AtProtocolInterface::ChatBskyConvoDeleteMessageForSelf;
using AtProtocolInterface::ChatBskyConvoGetConvo;
using AtProtocolInterface::ChatBskyConvoGetConvoForMembers;
using AtProtocolInterface::ChatBskyConvoGetLog;
using AtProtocolInterface::ChatBskyConvoGetMessages;
using AtProtocolInterface::ChatBskyConvoListConvos;
using AtProtocolInterface::ChatBskyConvoSendMessage;
using namespace AtProtocolType::ChatBskyConvoDefs;
using namespace AtProtocolType;

ChatMessageListModel::ChatMessageListModel(QObject *parent)
    : AtpChatAbstractListModel { parent }, m_runSending(false), m_ready(false)
{
    ChatLogSubscriber *log = ChatLogSubscriber::getInstance();
    connect(log, &ChatLogSubscriber::receiveLogs, this, &ChatMessageListModel::receiveLogs);
    connect(log, &ChatLogSubscriber::errorOccured, this, &ChatMessageListModel::errorLogs);
}

ChatMessageListModel::~ChatMessageListModel()
{
    ChatLogSubscriber *log = ChatLogSubscriber::getInstance();
    log->stop(account());
    disconnect(log, &ChatLogSubscriber::receiveLogs, this, &ChatMessageListModel::receiveLogs);
    disconnect(log, &ChatLogSubscriber::errorOccured, this, &ChatMessageListModel::errorLogs);
}

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
    else if (role == SenderDidRole)
        return current.sender.did;
    else if (role == SenderAvatarRole) {
        for (const auto &member : m_convo.members) {
            if (member.did == current.sender.did) {
                return member.avatar;
            }
        }
        return QString();
    } else if (role == TextRole)
        return LexiconsTypeUnknown::applyFacetsTo(current.text, current.facets);
    else if (role == SentAtRole)
        return LexiconsTypeUnknown::formatDateTime(current.sentAt);

    else if (role == RunningRole)
        return m_itemRunningHash.value(m_idList.at(row), false);

    return QVariant();
}

void ChatMessageListModel::update(int row, ChatMessageListModelRoles role, const QVariant &value)
{
    if (row < 0 || row >= m_idList.count())
        return;

    auto &current = m_messageHash[m_idList.at(row)];

    if (role == RunningRole) {
        bool running = m_itemRunningHash.value(m_idList.at(row), false);
        if (running != value.toBool()) {
            m_itemRunningHash[m_idList.at(row)] = value.toBool();
            emit dataChanged(index(row), index(row));
        }
    }
}

bool ChatMessageListModel::getLatest()
{
    if (running() || (convoId().isEmpty() && memberDids().isEmpty()))
        return false;
    setRunning(true);

    getServiceEndpoint([=]() {
        getConvo(convoId(), memberDids(), [=]() {
            getLogCursor([=] {
                if (m_idList.isEmpty() || m_logCursor.isEmpty()) {
                    ChatBskyConvoGetMessages *messages = new ChatBskyConvoGetMessages(this);
                    connect(messages, &ChatBskyConvoGetMessages::finished, this, [=](bool success) {
                        if (success) {
                            if (m_idList.isEmpty() && m_cursor.isEmpty()) {
                                m_cursor = messages->cursor();
                            }
                            copyFrom(messages->messagesMessageViewList(),
                                     messages->messagesDeletedMessageViewList(), true);

                            updateRead(convoId(), QString());
                            setReady(true);
                        } else {
                            emit errorOccured(messages->errorCode(), messages->errorMessage());
                            checkScopeError(messages->errorCode(), messages->errorMessage());
                        }
                        setRunning(false);
                        messages->deleteLater();
                    });
                    messages->setAccount(account());
                    messages->setService(account().service_endpoint);
                    messages->getMessages(convoId(), 0, QString());
                } else {
                    ChatLogSubscriber *log = ChatLogSubscriber::getInstance();
                    log->setAccount(account());
                    if (autoLoading()) {
                        log->start(account(), m_logCursor);
                    }
                    setRunning(false);
                }
            });
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
    if (runSending() || convoId().isEmpty() || message.trimmed().isEmpty())
        return;
    setRunSending(true);

    LexiconsTypeUnknown::makeFacets(
            this, account(), message.trimmed(),
            [=](const QList<AtProtocolType::AppBskyRichtextFacet::Main> &facets) {
                QJsonObject obj;
                obj.insert("text", message.trimmed());
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
                    finishSent(success);
                    setRunSending(false);
                    convo->deleteLater();
                });
                convo->setAccount(account());
                convo->setService(account().service_endpoint);
                convo->sendMessage(convoId(), obj);
            });
}

void ChatMessageListModel::deleteMessage(int row)
{
    if (row < 0 || row >= m_idList.count())
        return;
    const auto &current = m_messageHash[m_idList.at(row)];
    bool running = item(row, RunningRole).toBool();
    if (running)
        return;
    update(row, RunningRole, true);

    ChatBskyConvoDeleteMessageForSelf *convo = new ChatBskyConvoDeleteMessageForSelf(this);
    connect(convo, &ChatBskyConvoDeleteMessageForSelf::finished, this, [=](bool success) {
        if (success) {
        } else {
            emit errorOccured(convo->errorCode(), convo->errorMessage());
        }
        update(row, RunningRole, false);
        convo->deleteLater();
    });
    convo->setAccount(account());
    convo->setService(account().service_endpoint);
    convo->deleteMessageForSelf(convoId(), current.id);
}

QHash<int, QByteArray> ChatMessageListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[IdRole] = "id";
    roles[RevRole] = "rev";

    roles[SenderDidRole] = "senderDid";
    roles[SenderAvatarRole] = "senderAvatar";

    roles[TextRole] = "text";
    roles[SentAtRole] = "sentAt";

    roles[RunningRole] = "running";

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

void ChatMessageListModel::getConvo(const QString &convoId, const QStringList &memberDids,
                                    std::function<void()> callback)
{
    if ((convoId.isEmpty() && memberDids.isEmpty()) || !m_convo.id.isEmpty()) {
        callback();
        return;
    }

    if (!convoId.isEmpty()) {
        ChatBskyConvoGetConvo *convo = new ChatBskyConvoGetConvo(this);
        connect(convo, &ChatBskyConvoGetConvo::finished, [=](bool success) {
            if (success) {
                m_convo = convo->convo();
            } else {
                m_convo.id.clear();
                emit errorOccured(convo->errorCode(), convo->errorMessage());
                checkScopeError(convo->errorCode(), convo->errorMessage());
            }
            callback();
            convo->deleteLater();
        });
        convo->setAccount(account());
        convo->setService(account().service_endpoint);
        convo->getConvo(convoId);
    } else {
        ChatBskyConvoGetConvoForMembers *convo = new ChatBskyConvoGetConvoForMembers(this);
        connect(convo, &ChatBskyConvoGetConvoForMembers::finished, [=](bool success) {
            qDebug().noquote() << success << convo->replyJson();
            if (success) {
                m_convo = convo->convo();
                setConvoId(m_convo.id);
            } else {
                m_convo.id.clear();
                emit errorOccured(convo->errorCode(), convo->errorMessage());
                checkScopeError(convo->errorCode(), convo->errorMessage());
            }
            qDebug() << "in" << m_convo.id;
            callback();
            convo->deleteLater();
        });
        convo->setAccount(account());
        convo->setService(account().service_endpoint);
        convo->getConvoForMembers(memberDids);
    }
}

void ChatMessageListModel::getLogCursor(std::function<void()> callback)
{
    if (!m_logCursor.isEmpty()) {
        callback();
        return;
    }
    ChatBskyConvoListConvos *convos = new ChatBskyConvoListConvos(this);
    connect(convos, &ChatBskyConvoListConvos::finished, this, [=](bool success) {
        if (success && !convos->convosList().isEmpty()) {
            m_logCursor = convos->convosList().first().rev;
        } else {
            m_logCursor.clear();
        }
        qDebug() << "Convo id" << convoId() << "Log cursor" << m_logCursor;
        callback();
        convos->deleteLater();
    });
    convos->setAccount(account());
    convos->setService(account().service_endpoint);
    convos->listConvos(1, QString());
}

QString ChatMessageListModel::convoId() const
{
    return m_convoId;
}

void ChatMessageListModel::setConvoId(const QString &newConvoId)
{
    if (!m_convoId.isEmpty() || m_convoId == newConvoId)
        return;
    m_convoId = newConvoId;
    emit convoIdChanged();
}

bool ChatMessageListModel::runSending() const
{
    return m_runSending;
}

void ChatMessageListModel::setRunSending(bool newRunSending)
{
    if (m_runSending == newRunSending)
        return;
    m_runSending = newRunSending;
    emit runSendingChanged();
}

QStringList ChatMessageListModel::memberDids() const
{
    return m_memberDids;
}

void ChatMessageListModel::setMemberDids(const QStringList &newMemberDids)
{
    if (m_memberDids == newMemberDids)
        return;
    m_memberDids = newMemberDids;
    emit memberDidsChanged();
}

bool ChatMessageListModel::ready() const
{
    return m_ready;
}

void ChatMessageListModel::setReady(bool newReady)
{
    if (m_ready == newReady)
        return;
    m_ready = newReady;
    emit readyChanged();
}

void ChatMessageListModel::receiveLogs(const QString &key,
                                       const AtProtocolInterface::ChatBskyConvoGetLog &log)
{
    qDebug().quote() << "receiveLogs" << this << key << log.logsLogCreateMessageList().length();
    if (!ChatLogSubscriber::isMine(account(), key))
        return;

    // 別カラムで同一アカウントのチャットを閉じると再スタートさせるので、そのときにチャットを開いたときの古いものをしようしないようにする
    if (!log.cursor().isEmpty())
        m_logCursor = log.cursor();

    // old->new から new->oldの順番に直す
    QList<AtProtocolType::ChatBskyConvoDefs::MessageView> messages;
    for (const auto &msg : log.logsLogCreateMessageList()) {
        if (msg.message_type == LogCreateMessageMessageType::message_MessageView
            && msg.convoId == convoId()) {
            messages.insert(0, msg.message_MessageView);
        }
    }
    QList<AtProtocolType::ChatBskyConvoDefs::DeletedMessageView> deleted_messages;
    for (const auto &msg : log.logsLogDeleteMessageList()) {
        if (msg.message_type == LogDeleteMessageMessageType::message_DeletedMessageView
            && msg.convoId == convoId()) {
            deleted_messages.insert(0, msg.message_DeletedMessageView);
        }
    }

    copyFrom(messages, deleted_messages, true);
}

void ChatMessageListModel::errorLogs(const QString &key, const QString &code,
                                     const QString &message)
{
    qDebug().quote() << "errorLogs" << this << key << code << message;
    ChatLogSubscriber *log = ChatLogSubscriber::getInstance();
    if (!log->isMine(account(), key))
        return;

    emit errorOccured(code, message);
}
