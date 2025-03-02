#include "chatnotificationlistmodel.h"
#include "tools/accountmanager.h"
#include "atprotocol/chat/bsky/convo/chatbskyconvolistconvos.h"

using AtProtocolInterface::ChatBskyConvoListConvos;

ChatNotificationListModel::ChatNotificationListModel(QObject *parent)
    : AtpChatAbstractListModel { parent }, m_enabled(false)
{
}

int ChatNotificationListModel::rowCount(const QModelIndex &parent) const
{
    return m_chatNotificationData.count();
}

QVariant ChatNotificationListModel::data(const QModelIndex &index, int role) const
{
    return item(index.row(), static_cast<ChatNotificationListModelRoles>(role));
}

QVariant ChatNotificationListModel::item(int row, ChatNotificationListModelRoles role) const
{
    if (row < 0 || row >= m_chatNotificationData.count())
        return QVariant();

    const auto &current = m_chatNotificationData.at(row);

    switch (role) {
    case AccountUuidRole:
        return current.account_uuid;
    case AvatarRole:
        return current.avatar;
    case UnreadCountRole:
        return current.unread_count;
    case VisibleRole:
        return current.visible;
    default:
        break;
    }

    return QVariant();
}

bool ChatNotificationListModel::getLatest()
{
    if (running())
        return false;

    const auto a = AccountManager::getInstance();
    if (!a->allAccountsReady())
        return false;

    m_accountUuidCue = a->getUuids();
    if (m_accountUuidCue.isEmpty())
        return false;

    getChatList();
    return true;
}

bool ChatNotificationListModel::getNext()
{
    return true;
}

void ChatNotificationListModel::start()
{
    setLoadingInterval(60 * 1000);
    setAutoLoading(true);
}

void ChatNotificationListModel::hideItem(int row)
{
    if (row < 0 || row >= m_chatNotificationData.count())
        return;

    m_chatNotificationData[row].visible = false;

    emit dataChanged(index(row), index(row));
}

QHash<int, QByteArray> ChatNotificationListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[AccountUuidRole] = "accountUuid";
    roles[AvatarRole] = "avatar";
    roles[UnreadCountRole] = "unreadCount";
    roles[VisibleRole] = "visible";

    return roles;
}

void ChatNotificationListModel::getChatList()
{
    setRunning(true);

    if (m_accountUuidCue.isEmpty()) {
        setRunning(false);
        return;
    }
    const QString uuid = m_accountUuidCue.first();
    m_accountUuidCue.pop_front();

    const auto account = AccountManager::getInstance()->getAccount(uuid);
    if (!account.isValid()) {
        getChatList();
        return;
    }

    // getServiceEndpoint([=]() {
    // updateContentFilterLabels([=]() {
    ChatBskyConvoListConvos *convos = new ChatBskyConvoListConvos(this);
    connect(convos, &ChatBskyConvoListConvos::finished, this, [=](bool success) {
        if (success) {
            // if (m_idList.isEmpty() && m_cursor.isEmpty()) {
            //     m_cursor = convos->cursor();
            // }

            ChatNotificationData data;
            data.account_uuid = account.uuid;
            data.avatar = account.avatar;
            data.unread_count = 0;
            data.visible = true;
            for (auto item = convos->convosList().cbegin(); item != convos->convosList().cend();
                 item++) {
                data.unread_count += item->unreadCount;
            }
            if (data.unread_count > 0) {
                appendData(data);
            }
        } else {
            emit errorOccured(convos->errorCode(), convos->errorMessage());
            checkScopeError(convos->errorCode(), convos->errorMessage());
        }

        QTimer::singleShot(10, this, &ChatNotificationListModel::getChatList);
        convos->deleteLater();
    });
    convos->setAccount(account);
    convos->setService(account.service_endpoint);
    convos->setLabelers(labelerDids());
    convos->listConvos(0, QString(), "unread", QString());
    // });
    // });
}

void ChatNotificationListModel::appendData(const ChatNotificationData &data)
{
    int row = -1;
    for (int i = 0; i < m_chatNotificationData.count(); i++) {
        if (m_chatNotificationData.at(i).account_uuid == data.account_uuid) {
            row = i;
            break;
        }
    }
    if (row >= 0) {
        if (m_chatNotificationData.at(row).visible) {
            // 表示中
        } else if (m_chatNotificationData[row].unread_count < data.unread_count) {
            // 増えたら再表示
            m_chatNotificationData[row].visible = true;
        } else {
            // 減ったらそのまま
        }
        m_chatNotificationData[row].avatar = data.avatar;
        m_chatNotificationData[row].unread_count = data.unread_count;
        emit dataChanged(index(row), index(row));
    } else {
        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        m_chatNotificationData.append(data);
        endInsertRows();
    }
}

bool ChatNotificationListModel::enabled() const
{
    return m_enabled;
}

void ChatNotificationListModel::setEnabled(bool newEnabled)
{
    qDebug() << __FUNCTION__ << m_enabled << "->" << newEnabled;
    if (m_enabled == newEnabled)
        return;
    m_enabled = newEnabled;
    emit enabledChanged();

    if (m_enabled) {
        // 一度クリックして非表示にしたものもいったん表示に戻す
        for (int i = 0; i < m_chatNotificationData.count(); i++) {
            m_chatNotificationData[i].visible = true;
        }
        getLatest();
        setLoadingInterval(30 * 1000);
        setAutoLoading(true);
    } else {
        setAutoLoading(false);
    }
}
