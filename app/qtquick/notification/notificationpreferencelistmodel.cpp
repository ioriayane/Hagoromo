#include "notificationpreferencelistmodel.h"
#include "atprotocol/app/bsky/notification/appbskynotificationgetpreferences.h"
#include "atprotocol/app/bsky/notification/appbskynotificationputpreferencesv2.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

NotificationPreferenceListModel::NotificationPreferenceListModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_running(false)
    , m_modified(false)
{
    setupPreferenceItems();
}

int NotificationPreferenceListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_preferenceItems.size();
}

QVariant NotificationPreferenceListModel::data(const QModelIndex &index, int role) const
{
    return item(index.row(), static_cast<PreferenceRoles>(role));
}

QVariant NotificationPreferenceListModel::item(int row, PreferenceRoles role) const
{
    if (row < 0 || row >= m_preferenceItems.size()) {
        return QVariant();
    }

    const PreferenceItem &item = m_preferenceItems.at(row);

    switch (role) {
    case TypeRole:
        return static_cast<int>(item.type);
    case DisplayNameRole:
        return item.displayName;
    case IncludeRole:
        return item.include;
    case ListRole:
        return item.list;
    case PushRole:
        return item.push;
    case CategoryRole:
        switch (item.category) {
        case SocialCategory:
            return tr("Social");
        case InteractionCategory:
            return tr("Interaction");
        case SystemCategory:
            return tr("System");
        case ActivityCategory:
            return tr("Activity");
        default:
            return tr("Other");
        }
    case EnabledRole:
        return item.enabled;
    case DescriptionRole:
        return tr("Configure notification settings for %1").arg(item.displayName);
    case IncludeTypeRole:
        return static_cast<int>(item.includeType);
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> NotificationPreferenceListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TypeRole] = "type";
    roles[DisplayNameRole] = "displayName";
    roles[IncludeRole] = "include";
    roles[ListRole] = "list";
    roles[PushRole] = "push";
    roles[CategoryRole] = "category";
    roles[EnabledRole] = "enabled";
    roles[DescriptionRole] = "description";
    roles[IncludeTypeRole] = "includeType";
    return roles;
}

void NotificationPreferenceListModel::setAccount(const QString &uuid)
{
    if (m_account.uuid == uuid) {
        return;
    }
    m_account.uuid = uuid;
    emit accountChanged();
}

AtProtocolInterface::AccountData NotificationPreferenceListModel::getAccountData() const
{
    return AccountManager::getInstance()->getAccount(m_account.uuid);
}

void NotificationPreferenceListModel::updateInclude(int index, const QString &include)
{
    update(index, IncludeRole, include);
}

void NotificationPreferenceListModel::updateList(int index, bool list)
{
    update(index, ListRole, list);
}

void NotificationPreferenceListModel::updatePush(int index, bool push)
{
    update(index, PushRole, push);
}

void NotificationPreferenceListModel::loadPreferences()
{
    if (m_running) {
        return;
    }

    if (m_account.uuid.isEmpty()) {
        emit errorOccurred(tr("Account must be set"));
        return;
    }

    setRunning(true);

    AtProtocolInterface::AppBskyNotificationGetPreferences *getPreferences = 
            new AtProtocolInterface::AppBskyNotificationGetPreferences(this);
    connect(getPreferences, &AtProtocolInterface::AppBskyNotificationGetPreferences::finished,
            [=](bool success) {
                setRunning(false);

                if (!success) {
                    emit errorOccurred(tr("Failed to load notification preferences"));
                    getPreferences->deleteLater();
                    return;
                }

                const auto &prefs = getPreferences->preferences();
                updateFromAtProtocolPreferences(prefs);
                setModified(false);  // ロード完了後はmodifiedをfalseに
                emit preferencesUpdated();
                getPreferences->deleteLater();
            });
    
    auto accountData = getAccountData();
    getPreferences->setAccount(accountData);

    getPreferences->getPreferences();
}

void NotificationPreferenceListModel::savePreferences()
{
    if (m_running) {
        return;
    }

    if (m_account.uuid.isEmpty()) {
        emit errorOccurred(tr("Account must be set"));
        return;
    }

    setRunning(true);

    AtProtocolInterface::AppBskyNotificationPutPreferencesV2 *putPreferences = 
            new AtProtocolInterface::AppBskyNotificationPutPreferencesV2(this);
    connect(putPreferences, &AtProtocolInterface::AppBskyNotificationPutPreferencesV2::finished,
            [=](bool success) {
                setRunning(false);

                if (!success) {
                    emit errorOccurred(tr("Failed to save notification preferences"));
                    putPreferences->deleteLater();
                    return;
                }

                setModified(false);  // 保存完了後はmodifiedをfalseに
                emit preferencesUpdated();
                putPreferences->deleteLater();
            });
    
    auto accountData = getAccountData();
    putPreferences->setAccount(accountData);

    // 各設定項目をJSONオブジェクトに変換
    QJsonObject chat, follow, like, likeViaRepost, mention, quote, reply, repost, repostViaRepost;
    QJsonObject starterpackJoined, subscribedPost, unverified, verified;

    for (const auto &item : m_preferenceItems) {
        QJsonObject json = createPreferenceJson(item);
        
        switch (item.type) {
        case ChatType:
            chat = json;
            break;
        case FollowType:
            follow = json;
            break;
        case LikeType:
            like = json;
            break;
        case LikeViaRepostType:
            likeViaRepost = json;
            break;
        case MentionType:
            mention = json;
            break;
        case QuoteType:
            quote = json;
            break;
        case ReplyType:
            reply = json;
            break;
        case RepostType:
            repost = json;
            break;
        case RepostViaRepostType:
            repostViaRepost = json;
            break;
        case StarterpackJoinedType:
            starterpackJoined = json;
            break;
        case SubscribedPostType:
            subscribedPost = json;
            break;
        case UnverifiedType:
            unverified = json;
            break;
        case VerifiedType:
            verified = json;
            break;
        }
    }

    putPreferences->putPreferencesV2(chat, follow, like, likeViaRepost, mention, quote,
                                     reply, repost, repostViaRepost, starterpackJoined,
                                     subscribedPost, unverified, verified);
}

QString NotificationPreferenceListModel::getIncludeDisplayName(const QString &include) const
{
    if (include == "all") {
        return tr("All");
    } else if (include == "follows") {
        return tr("Follows only");
    } else if (include == "accepted") {
        return tr("Accepted only");
    }
    return include;
}

QStringList NotificationPreferenceListModel::getAvailableIncludeOptions(int type) const
{
    PreferenceType prefType = static_cast<PreferenceType>(type);
    
    // 対応するPreferenceItemを見つける
    for (const auto &item : m_preferenceItems) {
        if (item.type == prefType) {
            switch (item.includeType) {
            case AcceptedInclude:
                return { "all", "accepted" };
            case FollowsInclude:
                return { "all", "follows" };
            case NoInclude:
            default:
                return {};  // include設定なし
            }
        }
    }
    
    return {};  // 見つからない場合は空のリスト
}

void NotificationPreferenceListModel::setRunning(bool running)
{
    if (m_running == running) {
        return;
    }
    m_running = running;
    emit runningChanged();
}

void NotificationPreferenceListModel::setModified(bool modified)
{
    if (m_modified == modified) {
        return;
    }
    m_modified = modified;
    emit modifiedChanged();
}

void NotificationPreferenceListModel::setupPreferenceItems()
{
    m_preferenceItems.clear();


    // ソーシャルカテゴリ
    m_preferenceItems.append({
        FollowType,
        tr("Follow"),
        "all",
        true,
        true,
        SocialCategory,
        FollowsInclude,  // follows/allの選択
        true  // enabled
    });

    m_preferenceItems.append({
        LikeType,
        tr("Like"),
        "all",
        true,
        true,
        SocialCategory,
        FollowsInclude,  // follows/allの選択
        true  // enabled
    });

    m_preferenceItems.append({
        RepostType,
        tr("Repost"),
        "all",
        true,
        true,
        SocialCategory,
        FollowsInclude,  // follows/allの選択
        true  // enabled
    });

    m_preferenceItems.append({
        LikeViaRepostType,
        tr("Like via Repost"),
        "all",
        true,
        true,
        SocialCategory,
        FollowsInclude,  // follows/allの選択
        true  // enabled
    });

    m_preferenceItems.append({
        RepostViaRepostType,
        tr("Repost via Repost"),
        "all",
        true,
        true,
        SocialCategory,
        FollowsInclude,  // follows/allの選択
        true  // enabled
    });

    m_preferenceItems.append({
        ChatType,
        tr("Chat"),
        "all",
        false,
        false,
        SocialCategory, 
        AcceptedInclude,  // accepted/allの選択
        false  // enabled
    });

    // インタラクションカテゴリ
    m_preferenceItems.append({
        ReplyType,
        tr("Reply"),
        "all",
        true,
        true,
        InteractionCategory,
        FollowsInclude,  // follows/allの選択
        true  // enabled
    });

    m_preferenceItems.append({
        MentionType,
        tr("Mention"),
        "all",
        true,
        true,
        InteractionCategory,
        FollowsInclude,  // follows/allの選択
        true  // enabled
    });

    m_preferenceItems.append({
        QuoteType,
        tr("Quote"),
        "all",
        true,
        true,
        InteractionCategory,
        FollowsInclude,  // follows/allの選択
        true  // enabled
    });

    // アクティビティカテゴリ
    m_preferenceItems.append({
        StarterpackJoinedType,
        tr("Starterpack Joined"),
        "",
        true,
        true,
        ActivityCategory,
        NoInclude,  // include設定なし
        true  // enabled
    });

    m_preferenceItems.append({
        SubscribedPostType,
        tr("Subscribed Post"),
        "",
        true,
        true,
        ActivityCategory,
        NoInclude,  // include設定なし
        true  // enabled
    });

    // システムカテゴリ
    m_preferenceItems.append({
        UnverifiedType,
        tr("Unverified"),
        "",
        true,
        true,
        SystemCategory,
        NoInclude,  // include設定なし
        true  // enabled
    });

    m_preferenceItems.append({
        VerifiedType,
        tr("Verified"),
        "",
        true,
        true,
        SystemCategory,
        NoInclude,  // include設定なし
        true  // enabled
    });
}

void NotificationPreferenceListModel::updateFromAtProtocolPreferences(
        const AtProtocolType::AppBskyNotificationDefs::Preferences &prefs)
{
    beginResetModel();

    for (auto &item : m_preferenceItems) {
        switch (item.type) {
        case ChatType:
            item.include = prefs.chat.include;
            item.push = prefs.chat.push;
            break;
        case FollowType:
            item.include = prefs.follow.include;
            item.list = prefs.follow.list;
            item.push = prefs.follow.push;
            break;
        case LikeType:
            item.include = prefs.like.include;
            item.list = prefs.like.list;
            item.push = prefs.like.push;
            break;
        case LikeViaRepostType:
            item.include = prefs.likeViaRepost.include;
            item.list = prefs.likeViaRepost.list;
            item.push = prefs.likeViaRepost.push;
            break;
        case MentionType:
            item.include = prefs.mention.include;
            item.list = prefs.mention.list;
            item.push = prefs.mention.push;
            break;
        case QuoteType:
            item.include = prefs.quote.include;
            item.list = prefs.quote.list;
            item.push = prefs.quote.push;
            break;
        case ReplyType:
            item.include = prefs.reply.include;
            item.list = prefs.reply.list;
            item.push = prefs.reply.push;
            break;
        case RepostType:
            item.include = prefs.repost.include;
            item.list = prefs.repost.list;
            item.push = prefs.repost.push;
            break;
        case RepostViaRepostType:
            item.include = prefs.repostViaRepost.include;
            item.list = prefs.repostViaRepost.list;
            item.push = prefs.repostViaRepost.push;
            break;
        case StarterpackJoinedType:
            item.list = prefs.starterpackJoined.list;
            item.push = prefs.starterpackJoined.push;
            break;
        case SubscribedPostType:
            item.list = prefs.subscribedPost.list;
            item.push = prefs.subscribedPost.push;
            break;
        case UnverifiedType:
            item.list = prefs.unverified.list;
            item.push = prefs.unverified.push;
            break;
        case VerifiedType:
            item.list = prefs.verified.list;
            item.push = prefs.verified.push;
            break;
        }
    }

    endResetModel();
}

QJsonObject NotificationPreferenceListModel::createPreferenceJson(const PreferenceItem &item) const
{
    QJsonObject json;
    
    if (item.includeType != NoInclude) {
        json["include"] = item.include;
    }
    
    // chatPreferenceにはlistフィールドがない
    if (item.type != ChatType) {
        json["list"] = item.list;
    }
    
    json["push"] = item.push;
    
    return json;
}

bool NotificationPreferenceListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_preferenceItems.size()) {
        return false;
    }

    PreferenceItem &item = m_preferenceItems[index.row()];
    bool changed = false;

    switch (static_cast<PreferenceRoles>(role)) {
    case IncludeRole:
        if (item.include != value.toString()) {
            item.include = value.toString();
            changed = true;
        }
        break;
    case ListRole:
        if (item.list != value.toBool()) {
            item.list = value.toBool();
            changed = true;
        }
        break;
    case PushRole:
        if (item.push != value.toBool()) {
            item.push = value.toBool();
            changed = true;
        }
        break;
    case EnabledRole:
        if (item.enabled != value.toBool()) {
            item.enabled = value.toBool();
            changed = true;
        }
        break;
    default:
        return false;
    }

    if (changed) {
        setModified(true);
        emit dataChanged(index, index, { role });
        return true;
    }

    return false;
}

void NotificationPreferenceListModel::update(int row, PreferenceRoles role, const QVariant &value)
{
    if (row < 0 || row >= m_preferenceItems.size()) {
        return;
    }

    PreferenceItem &item = m_preferenceItems[row];
    bool changed = false;

    switch (role) {
    case IncludeRole:
        if (item.includeType != NoInclude && item.include != value.toString()) {
            item.include = value.toString();
            changed = true;
        }
        break;
    case ListRole:
        if (item.list != value.toBool()) {
            item.list = value.toBool();
            changed = true;
        }
        break;
    case PushRole:
        if (item.push != value.toBool()) {
            item.push = value.toBool();
            changed = true;
        }
        break;
    case EnabledRole:
        if (item.enabled != value.toBool()) {
            item.enabled = value.toBool();
            changed = true;
        }
        break;
    default:
        return;
    }

    if (changed) {
        setModified(true);
        QModelIndex modelIndex = this->index(row);
        emit dataChanged(modelIndex, modelIndex, { role });
    }
}

Qt::ItemFlags NotificationPreferenceListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}
