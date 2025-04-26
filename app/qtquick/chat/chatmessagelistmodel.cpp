#include "chatmessagelistmodel.h"

#include "atprotocol/chat/bsky/convo/chatbskyconvodeletemessageforself.h"
#include "atprotocol/chat/bsky/convo/chatbskyconvogetconvo.h"
#include "atprotocol/chat/bsky/convo/chatbskyconvogetconvoformembers.h"
#include "atprotocol/chat/bsky/convo/chatbskyconvogetlog.h"
#include "atprotocol/chat/bsky/convo/chatbskyconvogetmessages.h"
#include "atprotocol/chat/bsky/convo/chatbskyconvolistconvos.h"
#include "atprotocol/chat/bsky/convo/chatbskyconvosendmessage.h"
#include "atprotocol/chat/bsky/convo/chatbskyconvoaddreaction.h"
#include "atprotocol/chat/bsky/convo/chatbskyconvoremovereaction.h"
#include "atprotocol/lexicons_func_unknown.h"

using AtProtocolInterface::ChatBskyConvoAddReaction;
using AtProtocolInterface::ChatBskyConvoDeleteMessageForSelf;
using AtProtocolInterface::ChatBskyConvoGetConvo;
using AtProtocolInterface::ChatBskyConvoGetConvoForMembers;
using AtProtocolInterface::ChatBskyConvoGetLog;
using AtProtocolInterface::ChatBskyConvoGetMessages;
using AtProtocolInterface::ChatBskyConvoListConvos;
using AtProtocolInterface::ChatBskyConvoRemoveReaction;
using AtProtocolInterface::ChatBskyConvoSendMessage;
using namespace AtProtocolType::ChatBskyConvoDefs;
using namespace AtProtocolType;

ChatMessageListModel::ChatMessageListModel(QObject *parent)
    : AtpChatAbstractListModel { parent },
      m_chatLogConnector(this),
      m_runSending(false),
      m_ready(false)
{
    connect(&m_chatLogConnector, &ChatLogConnector::receiveLogs, this,
            &ChatMessageListModel::receiveLogs);
    connect(&m_chatLogConnector, &ChatLogConnector::errorOccured, this,
            &ChatMessageListModel::errorLogs);
}

ChatMessageListModel::~ChatMessageListModel()
{
    disconnect(&m_chatLogConnector, &ChatLogConnector::receiveLogs, this,
               &ChatMessageListModel::receiveLogs);
    disconnect(&m_chatLogConnector, &ChatLogConnector::errorOccured, this,
               &ChatMessageListModel::errorLogs);
    ChatLogSubscriber *log = ChatLogSubscriber::getInstance();
    log->stop(account());
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
    const auto &view_record = current.embed_AppBskyEmbedRecord_View.record_ViewRecord;

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
    else if (role == TextPlainRole)
        return current.text;
    else if (role == SentAtRole)
        return LexiconsTypeUnknown::formatDateTime(current.sentAt);
    else if (role == ReactionEmojisRole) {
        QString emojis;
        for (const auto &reaction : current.reactions) {
            emojis.append(reaction.value);
        }
        return emojis;
    } else if (role == CanReactionRole) {
        int reaction_count = 0;
        for (const auto &reaction : current.reactions) {
            if (reaction.sender.did == account().did) {
                reaction_count++;
            }
        }
        return (reaction_count < 5);
    }

    else if (role == HasQuoteRecordRole)
        return (current.embed_type == MessageViewEmbedType::embed_AppBskyEmbedRecord_View
                && (current.embed_AppBskyEmbedRecord_View.record_type
                            == AppBskyEmbedRecord::ViewRecordType::record_ViewRecord
                    || current.embed_AppBskyEmbedRecord_View.record_type
                            == AppBskyEmbedRecord::ViewRecordType::record_ViewNotFound
                    || current.embed_AppBskyEmbedRecord_View.record_type
                            == AppBskyEmbedRecord::ViewRecordType::record_ViewBlocked));
    else if (role == QuoteRecordCidRole)
        return view_record.cid;
    else if (role == QuoteRecordUriRole)
        return view_record.uri;
    else if (role == QuoteRecordDisplayNameRole)
        return view_record.author.displayName;
    else if (role == QuoteRecordHandleRole)
        return view_record.author.handle;
    else if (role == QuoteRecordAvatarRole)
        return view_record.author.avatar;
    else if (role == QuoteRecordRecordTextRole)
        return LexiconsTypeUnknown::copyRecordText(view_record.value);
    else if (role == QuoteRecordIndexedAtRole)
        return LexiconsTypeUnknown::formatDateTime(view_record.indexedAt);
    else if (role == QuoteRecordEmbedImagesRole)
        return LexiconsTypeUnknown::copyImagesFromRecord(view_record,
                                                         LexiconsTypeUnknown::CopyImageType::Thumb);
    else if (role == QuoteRecordEmbedImagesFullRole)
        return LexiconsTypeUnknown::copyImagesFromRecord(
                view_record, LexiconsTypeUnknown::CopyImageType::FullSize);
    else if (role == QuoteRecordEmbedImagesAltRole)
        return LexiconsTypeUnknown::copyImagesFromRecord(view_record,
                                                         LexiconsTypeUnknown::CopyImageType::Alt);
    else if (role == QuoteRecordBlockedRole) {
        return (current.embed_AppBskyEmbedRecord_View.record_type
                        == AppBskyEmbedRecord::ViewRecordType::record_ViewNotFound
                || current.embed_AppBskyEmbedRecord_View.record_type
                        == AppBskyEmbedRecord::ViewRecordType::record_ViewBlocked);
        // ラベラーの情報を取得できるようにする
    } else if (role == QuoteFilterMatchedRole) {
        if (view_record.author.viewer.muted)
            return true;
        if (view_record.author.did == account().did) // 自分のポストはそのまま表示
            return false;
        if (getContentFilterStatus(view_record.author.labels, false)
            != ConfigurableLabelStatus::Show)
            return true;
        if (getContentFilterStatus(view_record.labels, false) != ConfigurableLabelStatus::Show)
            return true;
        if (getContentFilterStatus(view_record.labels, true) != ConfigurableLabelStatus::Show)
            return true;
        return false;

    } else if (role == RunningRole)
        return m_itemRunningHash.value(m_idList.at(row), false);

    return QVariant();
}

void ChatMessageListModel::update(int row, ChatMessageListModelRoles role, const QVariant &value)
{
    if (row < 0 || row >= m_idList.count())
        return;

    // auto &current = m_messageHash[m_idList.at(row)];

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
                                     messages->messagesDeletedMessageViewList(),
                                     QList<AtProtocolType::ChatBskyConvoDefs::MessageView>(), true);

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
                    messages->setLabelers(labelerDids());
                    messages->getMessages(convoId(), 0, QString());
                } else {
                    ChatLogSubscriber *log = ChatLogSubscriber::getInstance();
                    log->setAccount(account(), &m_chatLogConnector);
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
                         messages->messagesDeletedMessageViewList(),
                         QList<AtProtocolType::ChatBskyConvoDefs::MessageView>(), false);
            } else {
                emit errorOccured(messages->errorCode(), messages->errorMessage());
            }
            setRunning(false);
            messages->deleteLater();
        });
        messages->setAccount(account());
        messages->setService(account().service_endpoint);
        messages->setLabelers(labelerDids());
        messages->getMessages(convoId(), 0, m_cursor);
    });

    return true;
}

void ChatMessageListModel::send(const QString &message, const QString &embed_uri,
                                const QString &embed_cid)
{
    if (runSending() || convoId().isEmpty()
        || (message.trimmed().isEmpty() && embed_uri.isEmpty() && embed_cid.isEmpty()))
        return;
    setRunSending(true);

    LexiconsTypeUnknown::makeFacets(
            this, account(), message.trimmed(),
            [=](const QList<AtProtocolType::AppBskyRichtextFacet::Main> &facets) {
                QJsonObject obj;
                obj.insert("text", message.trimmed());
                LexiconsTypeUnknown::insertFacetsJson(obj, facets);
                if (!embed_uri.isEmpty() && !embed_cid.isEmpty()) {
                    QJsonObject json_quote;
                    json_quote.insert("cid", embed_cid);
                    json_quote.insert("uri", embed_uri);
                    QJsonObject json_embed;
                    json_embed.insert("$type", "app.bsky.embed.record");
                    json_embed.insert("record", json_quote);
                    obj.insert("embed", json_embed);
                }

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

void ChatMessageListModel::addReaction(int row, const QString &emoji)
{
    if (row < 0 || row >= m_idList.count())
        return;
    const auto &current = m_messageHash[m_idList.at(row)];
    bool running = item(row, RunningRole).toBool();
    if (running)
        return;
    update(row, RunningRole, true);

    ChatBskyConvoAddReaction *convo = new ChatBskyConvoAddReaction(this);
    connect(convo, &ChatBskyConvoAddReaction::finished, this, [=](bool success) {
        if (success) {
        } else {
            emit errorOccured(convo->errorCode(), convo->errorMessage());
        }
        update(row, RunningRole, false);
        convo->deleteLater();
    });
    convo->setAccount(account());
    convo->setService(account().service_endpoint);
    convo->addReaction(convoId(), current.id, emoji);
}

void ChatMessageListModel::removeReaction(int row, const QString &emoji)
{
    if (row < 0 || row >= m_idList.count())
        return;
    const auto &current = m_messageHash[m_idList.at(row)];
    bool running = item(row, RunningRole).toBool();
    if (running)
        return;
    update(row, RunningRole, true);

    ChatBskyConvoRemoveReaction *convo = new ChatBskyConvoRemoveReaction(this);
    connect(convo, &ChatBskyConvoRemoveReaction::finished, this, [=](bool success) {
        if (success) {
        } else {
            emit errorOccured(convo->errorCode(), convo->errorMessage());
        }
        update(row, RunningRole, false);
        convo->deleteLater();
    });
    convo->setAccount(account());
    convo->setService(account().service_endpoint);
    convo->removeReaction(convoId(), current.id, emoji);
}

QHash<int, QByteArray> ChatMessageListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[IdRole] = "id";
    roles[RevRole] = "rev";

    roles[SenderDidRole] = "senderDid";
    roles[SenderAvatarRole] = "senderAvatar";

    roles[TextRole] = "text";
    roles[TextPlainRole] = "textPlain";
    roles[SentAtRole] = "sentAt";
    roles[ReactionEmojisRole] = "reactionEmojis";
    roles[CanReactionRole] = "canReaction";

    roles[HasQuoteRecordRole] = "hasQuoteRecord";
    roles[QuoteRecordCidRole] = "quoteRecordCid";
    roles[QuoteRecordUriRole] = "quoteRecordUri";
    roles[QuoteRecordDisplayNameRole] = "quoteRecordDisplayName";
    roles[QuoteRecordHandleRole] = "quoteRecordHandle";
    roles[QuoteRecordAvatarRole] = "quoteRecordAvatar";
    roles[QuoteRecordRecordTextRole] = "quoteRecordRecordText";
    roles[QuoteRecordIndexedAtRole] = "quoteRecordIndexedAt";
    roles[QuoteRecordEmbedImagesRole] = "quoteRecordEmbedImages";
    roles[QuoteRecordEmbedImagesFullRole] = "quoteRecordEmbedImagesFull";
    roles[QuoteRecordEmbedImagesAltRole] = "quoteRecordEmbedImagesAlt";
    roles[QuoteRecordBlockedRole] = "quoteRecordBlocked";

    roles[QuoteFilterMatchedRole] = "quoteFilterMatched";

    roles[RunningRole] = "running";

    return roles;
}

void ChatMessageListModel::copyFrom(
        const QList<MessageView> &messages, const QList<DeletedMessageView> &deletedMessages,
        const QList<AtProtocolType::ChatBskyConvoDefs::MessageView> &removedReactions, bool to_top)
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

    for (auto item = removedReactions.crbegin(); item != removedReactions.crend(); item++) {
        int row = m_idList.indexOf(item->id);
        if (row >= 0) {
            // auto &msg = m_messageHash[item->id];
            // for (int i = 0; i < msg.reactions.length(); i++) {
            //     if (msg.reactions.at(i).sender.did == (*item).sender.did
            //         && msg.reactions.at(i).value == (*item).rev) {
            //         //
            //     }
            // }
            m_messageHash[item->id] = *item;
            emit dataChanged(index(row), index(row));
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
                setMembers(m_convo);
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
                setMembers(m_convo);
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
    convos->listConvos(1, QString(), QString(), QString());
}

void ChatMessageListModel::setMembers(const AtProtocolType::ChatBskyConvoDefs::ConvoView &convo)
{
    QStringList avatars;
    QStringList handles;
    QStringList displaynames;
    for (const auto &member : convo.members) {
        if (member.did == account().did)
            continue;
        avatars.append(member.avatar);
        handles.append(member.handle);
        displaynames.append(member.displayName);
    }
    setMemberAvatars(avatars);
    setMemberHandles(handles);
    setMemberDisplayNames(displaynames);
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

void ChatMessageListModel::receiveLogs(const AtProtocolInterface::ChatBskyConvoGetLog &log)
{
    qDebug().quote() << "receiveLogs" << this << account().did
                     << log.logsLogCreateMessageList().length()
                     << log.logsLogAddReactionList().length();

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
    for (const auto &reaction : log.logsLogAddReactionList()) {
        if (reaction.message_type == LogAddReactionMessageType::message_MessageView
            && reaction.convoId == convoId()) {
            messages.insert(0, reaction.message_MessageView);
        }
    }
    QList<AtProtocolType::ChatBskyConvoDefs::DeletedMessageView> deleted_messages;
    for (const auto &msg : log.logsLogDeleteMessageList()) {
        if (msg.message_type == LogDeleteMessageMessageType::message_DeletedMessageView
            && msg.convoId == convoId()) {
            deleted_messages.insert(0, msg.message_DeletedMessageView);
        }
    }
    QList<AtProtocolType::ChatBskyConvoDefs::MessageView> removedReactions;
    for (const auto &reaction : log.logsLogRemoveReactionList()) {
        if (reaction.message_type == LogRemoveReactionMessageType::message_MessageView
            && reaction.convoId == convoId()) {
            removedReactions.insert(0, reaction.message_MessageView);
        }
    }

    copyFrom(messages, deleted_messages, removedReactions, true);
}

void ChatMessageListModel::errorLogs(const QString &code, const QString &message)
{
    qDebug().quote() << "errorLogs" << this << account().did << code << message;

    emit errorOccured(code, message);
}

QStringList ChatMessageListModel::memberAvatars() const
{
    return m_memberAvatars;
}

void ChatMessageListModel::setMemberAvatars(const QStringList &newMemberAvatars)
{
    if (m_memberAvatars == newMemberAvatars)
        return;
    m_memberAvatars = newMemberAvatars;
    emit memberAvatarsChanged();
}

QStringList ChatMessageListModel::memberHandles() const
{
    return m_memberHandles;
}

void ChatMessageListModel::setMemberHandles(const QStringList &newMemberHandles)
{
    if (m_memberHandles == newMemberHandles)
        return;
    m_memberHandles = newMemberHandles;
    emit memberHandlesChanged();
}

QStringList ChatMessageListModel::memberDisplayNames() const
{
    return m_memberDisplayNames;
}

void ChatMessageListModel::setMemberDisplayNames(const QStringList &newMemberDisplayNames)
{
    if (m_memberDisplayNames == newMemberDisplayNames)
        return;
    m_memberDisplayNames = newMemberDisplayNames;
    emit memberDisplayNamesChanged();
}
