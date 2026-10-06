#include "realtimefeedlistmodel.h"

#include "atprotocol/app/bsky/feed/appbskyfeedgetposts.h"
#include "atprotocol/app/bsky/graph/appbskygraphgetfollows.h"
#include "atprotocol/app/bsky/graph/appbskygraphgetfollowers.h"
#include "atprotocol/app/bsky/graph/appbskygraphgetlist.h"

#include <QDateTime>

using namespace RealtimeFeed;
using AtProtocolInterface::AppBskyFeedGetPosts;
using AtProtocolInterface::AppBskyGraphGetFollowers;
using AtProtocolInterface::AppBskyGraphGetFollows;
using AtProtocolInterface::AppBskyGraphGetList;

RealtimeFeedListModel::RealtimeFeedListModel(QObject *parent)
    : TimelineListModel { parent }, m_runningCue(false), m_receiving(false)
{
    FirehoseReceiver *receiver = FirehoseReceiver::getInstance();
    connect(receiver, &FirehoseReceiver::receivingChanged, this,
            &RealtimeFeedListModel::setReceiving);

    m_reactionFlushTimer.setInterval(1000);
    connect(&m_reactionFlushTimer, &QTimer::timeout, this,
            &RealtimeFeedListModel::flushReactionCounts);
    m_reactionFlushTimer.start();
}

RealtimeFeedListModel::~RealtimeFeedListModel()
{
    FirehoseReceiver *receiver = FirehoseReceiver::getInstance();
    disconnect(receiver, &FirehoseReceiver::receivingChanged, this,
               &RealtimeFeedListModel::setReceiving);
    receiver->removeSelector(this);
}

bool RealtimeFeedListModel::getLatest()
{
    FirehoseReceiver *receiver = FirehoseReceiver::getInstance();
    if (receiver->containsSelector(this)) {
        qDebug().noquote() << "FirehoseReceiver::status :" << receiver->status()
                           << ", selectorIsReady :" << receiver->selectorIsReady(this);
        if (receiver->selectorIsReady(this)) {
            if (receiver->status() != FirehoseReceiver::FirehoseReceiverStatus::Connected) {
                qDebug().noquote() << "Restart firehose(stop)";
                receiver->stop();
            }
            qDebug().noquote() << "Restart firehose(start)";
            receiver->start();
        }
        setRunning(false);
        return true;
    }
    if (selectorJson().isEmpty()) {
        setRunning(false);
        return false;
    }
    QJsonObject json = QJsonDocument::fromJson(selectorJson().toUtf8()).object();
    if (json.isEmpty()) {
        setRunning(false);
        return false;
    }
    setRunning(true);

    AbstractPostSelector *selector = AbstractPostSelector::create(json, this);
    if (selector == nullptr) {
        setRunning(false);
        return false;
    }
    receiver->appendSelector(selector);

    selector->setDid(account().did);
    selector->setHandle(account().handle);
    selector->setDisplayName(account().displayName);
    connect(selector, &AbstractPostSelector::selected, this, [=](const QJsonObject &object) {
        // qDebug().noquote() << QJsonDocument(object).toJson();
        for (const auto &info : selector->getOperationInfos(object)) {
            if (info.action == OperationActionType::Create) {
                m_cueGetPosts.append(info);
            }
        }
        if (!m_cueGetPosts.isEmpty() && !m_runningCue) {
#ifdef QT_DEBUG
            {
                QDateTime date =
                        QDateTime::fromString(m_cueGetPosts.first().time, Qt::ISODateWithMs);
                qDebug().noquote()
                        << "DIFF" << QString::number(date.msecsTo(QDateTime::currentDateTimeUtc()));
            }
#endif
            getQueuedPosts();
        }
    });
    connect(selector, &AbstractPostSelector::reacted, this, [=](const QJsonObject &object) {
        const QList<OperationInfo> infos = selector->getOperationInfos(object, true);
        for (const auto &info : infos) {
#ifdef QT_DEBUG
            {
                QDateTime date = QDateTime::fromString(info.time, Qt::ISODateWithMs);
                qDebug().noquote()
                        << "DIFF" << QString::number(date.msecsTo(QDateTime::currentDateTimeUtc()));
            }
#endif
            const QList<int> rows = indexsOf(info.cid);
            if (rows.isEmpty()) {
                continue;
            }
            if (info.is_repost) {
                // 自分のリポスト状態はボタン表示に直結するので即時反映
                if (info.reacted_by_did == account().did) {
                    update(rows.first(), RepostedUriRole, info.reaction_uri);
                }
                // カウントはポストデータの実態が1つなので1回だけ更新し、
                // GUIへの通知(dataChanged)はflushReactionCounts()でまとめて行う
                if (!isReactionCountIncluded(info.cid, info.time)) {
                    updateReactionCount(info.cid, TimelineListModelRoles::RepostCountRole,
                                        info.action == OperationActionType::Create);
                }
            } else if (info.is_like) {
                if (info.reacted_by_did == account().did) {
                    update(rows.first(), LikedUriRole, info.reaction_uri);
                }
                if (!isReactionCountIncluded(info.cid, info.time)) {
                    updateReactionCount(info.cid, TimelineListModelRoles::LikeCountRole,
                                        info.action == OperationActionType::Create);
                }
            } else {
                // delete post
                qDebug().noquote() << "delete" << rows << info.uri << info.cid;
                for (auto it = rows.crbegin(); it != rows.crend(); ++it) {
                    beginRemoveRows(QModelIndex(), *it, *it);
                    m_cidList.removeAt(*it);
                    endRemoveRows();
                }
                m_dirtyReactionCountRoles.remove(info.cid);
                m_postFetchedTime.remove(info.cid);
            }
        }
    });

    m_followings.clear();
    m_followers.clear();
    m_list_members.clear();
    m_get_list_cue = selector->getListUris();
    if (selector->needFollowing()) {
        m_cursor = "___start___";
        getFollowing();
    } else if (selector->needFollowers()) {
        m_cursor = "___start___";
        getFollowers();
    } else if (selector->needListMembers()) {
        m_cursor = "___start___";
        getListMembers();
    } else {
        finishGetting(selector);
    }

    return true;
}

bool RealtimeFeedListModel::getNext()
{
    return true;
}

bool RealtimeFeedListModel::repost(int row)
{
    return TimelineListModel::repost(row, false);
}

bool RealtimeFeedListModel::like(int row)
{
    return TimelineListModel::like(row, false);
}

bool RealtimeFeedListModel::bookmark(int row)
{
    return TimelineListModel::bookmark(row);
}

QString RealtimeFeedListModel::selectorJson() const
{
    return m_selectorJson;
}

void RealtimeFeedListModel::setSelectorJson(const QString &newSelectorJson)
{
    if (m_selectorJson == newSelectorJson)
        return;
    m_selectorJson = newSelectorJson;
    emit selectorJsonChanged();
}

void RealtimeFeedListModel::getFollowing()
{
    AbstractPostSelector *selector = FirehoseReceiver::getInstance()->getSelector(this);
    if (selector == nullptr) {
        setRunning(false);
        return;
    }
    if (m_cursor.isEmpty()) {
        if (selector->needFollowers()) {
            m_cursor = "___start___";
            getFollowers();
        } else if (selector->needListMembers()) {
            m_cursor = "___start___";
            getListMembers();
        } else {
            finishGetting(selector);
        }
        return;
    } else if (m_cursor == "___start___") {
        m_cursor.clear();
    }

    AppBskyGraphGetFollows *profiles = new AppBskyGraphGetFollows(this);
    connect(profiles, &AppBskyGraphGetFollows::finished, this, [=](bool success) {
        m_cursor.clear();
        if (success) {
            if (!profiles->followsList().isEmpty()) {
                // copy DID and rkey
                copyFollows(profiles->followsList(), true);
                // next
                m_cursor = profiles->cursor();
            }
            QTimer::singleShot(0, this, &RealtimeFeedListModel::getFollowing);
        } else {
            emit errorOccurred(profiles->errorCode(), profiles->errorMessage());
            abortGetting();
        }
        profiles->deleteLater();
    });
    profiles->setAccount(account());
    profiles->getFollows(account().did, 100, m_cursor, "latest");
}

void RealtimeFeedListModel::getFollowers()
{
    AbstractPostSelector *selector = FirehoseReceiver::getInstance()->getSelector(this);
    if (selector == nullptr) {
        setRunning(false);
        return;
    }
    if (m_cursor.isEmpty()) {
        if (selector->needListMembers()) {
            m_cursor = "___start___";
            getListMembers();
        } else {
            finishGetting(selector);
        }
        return;
    } else if (m_cursor == "___start___") {
        m_cursor.clear();
    }

    AppBskyGraphGetFollowers *profiles = new AppBskyGraphGetFollowers(this);
    connect(profiles, &AppBskyGraphGetFollowers::finished, this, [=](bool success) {
        m_cursor.clear();
        if (success) {
            if (!profiles->followsList().isEmpty()) {
                // copy DID and rkey
                copyFollows(profiles->followsList(), false);
                // next
                m_cursor = profiles->cursor();
            }
            QTimer::singleShot(0, this, &RealtimeFeedListModel::getFollowers);
        } else {
            emit errorOccurred(profiles->errorCode(), profiles->errorMessage());
            abortGetting();
        }
        profiles->deleteLater();
    });
    profiles->setAccount(account());
    profiles->getFollowers(account().did, 100, m_cursor, "latest");
}

void RealtimeFeedListModel::getListMembers()
{
    // リストはキャッシュを使いたいが、キャッシュにはハンドルなどがないので普通に取得した方が早い
    // ルールの中にリストが複数ある場合があるので、各リストのセレクターごとに取得しないといけない

    AbstractPostSelector *selector = FirehoseReceiver::getInstance()->getSelector(this);
    if (selector == nullptr) {
        setRunning(false);
        return;
    }
    QString list_uri = m_get_list_cue.isEmpty() ? QString() : m_get_list_cue.first();
    if (m_cursor.isEmpty() || list_uri.isEmpty()) {
        if (!m_get_list_cue.isEmpty()) {
            m_get_list_cue.pop_front();
        }
        if (m_get_list_cue.isEmpty()) {
            finishGetting(selector);
            return;
        } else {
            // Next list
            list_uri = m_get_list_cue.first();
            m_cursor.clear();
        }
    } else if (m_cursor == "___start___") {
        m_cursor.clear();
    }
    AppBskyGraphGetList *list = new AppBskyGraphGetList(this);
    connect(list, &AppBskyGraphGetList::finished, this, [=](bool success) {
        m_cursor.clear();
        if (success) {
            if (!list->itemsList().isEmpty()) {
                // list members
                copyListMembers(list_uri, list->itemsList());
                // next
                m_cursor = list->cursor();
            }
            QTimer::singleShot(0, this, &RealtimeFeedListModel::getListMembers);
        } else {
            emit errorOccurred(list->errorCode(), list->errorMessage());
            abortGetting();
        }
        list->deleteLater();
    });
    list->setAccount(account());
    list->getList(list_uri, 100, m_cursor);
}

void RealtimeFeedListModel::finishGetting(RealtimeFeed::AbstractPostSelector *selector)
{
    setRunning(false);
    if (selector != nullptr) {
        // QStringList users;
        // for (const auto &item : m_followings) {
        //     users.append(item.did);
        // }
        // qDebug().noquote() << "Followings" << users;
        // users.clear();
        // for (const auto &item : m_followers) {
        //     users.append(item.did);
        // }
        // qDebug().noquote() << "Followers" << users;
        qDebug().noquote() << "Following count : " << m_followings.count();
        qDebug().noquote() << "Followers count : " << m_followers.count();
        selector->setFollowing(m_followings);
        selector->setFollowers(m_followers);
        for (const auto &list_uri : m_list_members.keys()) {
            qDebug().noquote() << "Members count : " << m_list_members.value(list_uri).count()
                               << list_uri;
            selector->setListMembers(list_uri, m_list_members.value(list_uri));
        }
        selector->setReady(true);
#ifdef HAGOROMO_UNIT_TEST
        qDebug().noquote()
                << "FirehoseReceiver::getInstance()->start() --- No start on unit test mode";
#else
        FirehoseReceiver::getInstance()->start();
#endif
    }
}

void RealtimeFeedListModel::abortGetting()
{
    // finishGetting() に到達する前にエラーが発生した場合、
    // selector を未完成のまま残さず取り除く。
    // これにより次回の getLatest() は containsSelector() が false になり、最初からやり直せる。
    FirehoseReceiver::getInstance()->removeSelector(this);
    setRunning(false);
}

void RealtimeFeedListModel::copyFollows(
        const QList<AtProtocolType::AppBskyActorDefs::ProfileView> &follows, bool is_following)
{
    for (const auto &follow : follows) {
        RealtimeFeed::UserInfo user;
        user.did = follow.did;
        user.handle = follow.handle;
        user.display_name = follow.displayName;
        QString uri;
        if (is_following) {
            uri = follow.viewer.following;
        } else {
            uri = follow.viewer.followedBy;
        }
        if (uri.startsWith("at://")) {
            user.rkey = uri.split("/").last();
            if (is_following) {
                m_followings.append(user);
            } else {
                m_followers.append(user);
            }
        }
    }
}

void RealtimeFeedListModel::copyListMembers(
        const QString &list_uri, const QList<AtProtocolType::AppBskyGraphDefs::ListItemView> &items)
{
    for (const auto &item : items) {
        if (item.subject) {
            RealtimeFeed::UserInfo user;
            user.did = item.subject->did;
            user.handle = item.subject->handle;
            user.display_name = item.subject->displayName;
            user.rkey = item.uri.split("/").last();
            m_list_members[list_uri].append(user);
        }
    }
}

void RealtimeFeedListModel::getQueuedPosts()
{
    if (m_cueGetPosts.isEmpty()) {
        // Tokimekiの投票を取得
        QTimer::singleShot(50, this, &RealtimeFeedListModel::getTokimekiPoll);
        m_runningCue = false;
        return;
    }

    // 1回のgetPostsで取得できる上限(25件)までキューからまとめて取り出す
    // リプライ先のポストも同じリクエストで取得する
    const int max_uris = 25;
    QList<RealtimeFeed::OperationInfo> ope_infos;
    QStringList uris;
    while (!m_cueGetPosts.isEmpty()) {
        const RealtimeFeed::OperationInfo &info = m_cueGetPosts.first();
        QStringList adding;
        if (!uris.contains(info.uri)) {
            adding.append(info.uri);
        }
        if (!info.reply_parent_uri.isEmpty() && !uris.contains(info.reply_parent_uri)
            && !adding.contains(info.reply_parent_uri)) {
            adding.append(info.reply_parent_uri);
        }
        if (!uris.isEmpty() && (uris.count() + adding.count()) > max_uris) {
            break;
        }
        uris.append(adding);
        ope_infos.append(m_cueGetPosts.takeFirst());
    }

    m_runningCue = true;

    AppBskyFeedGetPosts *posts = new AppBskyFeedGetPosts(this);
    connect(posts, &AppBskyFeedGetPosts::finished, this, [=](bool success) {
        if (success) {
            const qint64 fetched_time = QDateTime::currentMSecsSinceEpoch();
            QHash<QString, AtProtocolType::AppBskyFeedDefs::PostView> post_hash; // <uri, post>
            for (const auto &post : posts->postsList()) {
                post_hash[post.uri] = post;
            }
            for (const auto &ope_info : ope_infos) {
                if (!post_hash.contains(ope_info.uri)) {
                    // 削除済みなどで取得できなかった
                    qDebug().noquote() << "Not found:" << ope_info.uri;
                    continue;
                }
                AtProtocolType::AppBskyFeedDefs::FeedViewPost view_post;
                view_post.post = post_hash.value(ope_info.uri);
                if (ope_info.is_repost) {
                    view_post.reason_type = AtProtocolType::AppBskyFeedDefs::
                            FeedViewPostReasonType::reason_ReasonRepost;
                    view_post.reason_ReasonRepost.by.did = ope_info.reacted_by_did;
                    view_post.reason_ReasonRepost.by.handle = ope_info.reacted_by_handle;
                    view_post.reason_ReasonRepost.by.displayName = ope_info.reacted_by_display_name;
                    view_post.reason_ReasonRepost.cid = ope_info.reaction_cid;
                    view_post.reason_ReasonRepost.uri = ope_info.reaction_uri;
                }
                if (!ope_info.reply_parent_uri.isEmpty()
                    && post_hash.contains(ope_info.reply_parent_uri)) {
                    view_post.reply.parent_type =
                            AtProtocolType::AppBskyFeedDefs::ReplyRefParentType::parent_PostView;
                    view_post.reply.parent_PostView = post_hash.value(ope_info.reply_parent_uri);
                }
                m_viewPostHash[view_post.post.cid] = view_post;
                m_postFetchedTime[view_post.post.cid] = fetched_time;
                bool visible = checkVisibility(view_post.post.cid);
                if (visible) {
                    beginInsertRows(QModelIndex(), 0, 0);
                    m_cidList.insert(0, view_post.post.cid);
                    endInsertRows();

                    // Tokimekiの投票の取得のキューに入れる
                    appendTokimekiPollToCue(view_post.post.cid,
                                            view_post.post.embed_AppBskyEmbedExternal_View);
                }
                m_originalCidList.insert(0, view_post.post.cid);
            }
        } else {
            emit errorOccurred(posts->errorCode(), posts->errorMessage());
        }
        // 残ってたらもう1回
        QTimer::singleShot(0, this, &RealtimeFeedListModel::getQueuedPosts);
        posts->deleteLater();
    });
    posts->setAccount(account());
    posts->setLabelers(labelerDids());
    posts->getPosts(uris);
}

void RealtimeFeedListModel::updateReactionCount(const QString &cid,
                                                TimelineListModel::TimelineListModelRoles role,
                                                bool increment)
{
    if (!m_viewPostHash.contains(cid)) {
        return;
    }
    AtProtocolType::AppBskyFeedDefs::FeedViewPost &current = m_viewPostHash[cid];
    if (role == RepostCountRole) {
        current.post.repostCount += increment ? 1 : -1;
        if (current.post.repostCount < 0) {
            current.post.repostCount = 0;
        }
    } else if (role == LikeCountRole) {
        current.post.likeCount += increment ? 1 : -1;
        if (current.post.likeCount < 0) {
            current.post.likeCount = 0;
        }
    } else {
        return;
    }
    m_dirtyReactionCountRoles[cid].insert(role);
}

void RealtimeFeedListModel::flushReactionCounts()
{
    if (m_dirtyReactionCountRoles.isEmpty()) {
        return;
    }
    const QHash<QString, QSet<int>> dirty = m_dirtyReactionCountRoles;
    m_dirtyReactionCountRoles.clear();

    for (auto it = dirty.constBegin(); it != dirty.constEnd(); ++it) {
        const QList<int> rows = indexsOf(it.key());
        if (rows.isEmpty()) {
            continue;
        }
        QVector<int> roles;
        for (int role : it.value()) {
            roles.append(role);
        }
        for (const auto row : rows) {
            emit dataChanged(index(row), index(row), roles);
        }
    }
}

bool RealtimeFeedListModel::isReactionCountIncluded(const QString &cid, const QString &time) const
{
    if (!m_postFetchedTime.contains(cid)) {
        return false;
    }
    const QDateTime reacted_time = QDateTime::fromString(time, Qt::ISODateWithMs);
    if (!reacted_time.isValid()) {
        return false;
    }
    return reacted_time.toMSecsSinceEpoch() <= m_postFetchedTime.value(cid);
}

bool RealtimeFeedListModel::receiving() const
{
    return m_receiving;
}

void RealtimeFeedListModel::setReceiving(bool newReceiving)
{
    if (m_receiving == newReceiving)
        return;
    m_receiving = newReceiving;
    emit receivingChanged();
}
