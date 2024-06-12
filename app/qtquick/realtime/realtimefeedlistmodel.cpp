#include "realtimefeedlistmodel.h"

#include "atprotocol/app/bsky/feed/appbskyfeedgetposts.h"
#include "atprotocol/app/bsky/graph/appbskygraphgetfollows.h"
#include "atprotocol/app/bsky/graph/appbskygraphgetfollowers.h"

using namespace RealtimeFeed;
using AtProtocolInterface::AppBskyFeedGetPosts;
using AtProtocolInterface::AppBskyGraphGetFollowers;
using AtProtocolInterface::AppBskyGraphGetFollows;

RealtimeFeedListModel::RealtimeFeedListModel(QObject *parent) : TimelineListModel { parent } { }

RealtimeFeedListModel::~RealtimeFeedListModel()
{
    FirehoseReceiver::getInstance()->removeSelector(this);
}

bool RealtimeFeedListModel::getLatest()
{
    FirehoseReceiver *receiver = FirehoseReceiver::getInstance();
    if (receiver->containsSelector(this)) {
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
    connect(selector, &AbstractPostSelector::selected, this, [=](const QJsonObject &object) {
        // qDebug().noquote() << QJsonDocument(object).toJson();
        QStringList uris = AbstractPostSelector::getOperationUris(object);
        if (!uris.isEmpty()) {
            m_cueGetPost.append(uris);
            getPosts();
        }
    });

    m_followings.clear();
    m_followers.clear();
    if (selector->needFollowing()) {
        m_cursor = "___start___";
        getFollowing();
    } else if (selector->needFollowers()) {
        m_cursor = "___start___";
        getFollowers();
    } else {
        finishGetting(selector);
    }

    return true;
}

bool RealtimeFeedListModel::getNext()
{
    return true;
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
            emit errorOccured(profiles->errorCode(), profiles->errorMessage());
            setRunning(false);
        }
        profiles->deleteLater();
    });
    profiles->setAccount(account());
    profiles->getFollows(account().did, 100, m_cursor);
}

void RealtimeFeedListModel::getFollowers()
{
    AbstractPostSelector *selector = FirehoseReceiver::getInstance()->getSelector(this);
    if (selector == nullptr) {
        setRunning(false);
        return;
    }
    if (m_cursor.isEmpty()) {
        finishGetting(selector);
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
            emit errorOccured(profiles->errorCode(), profiles->errorMessage());
            setRunning(false);
        }
        profiles->deleteLater();
    });
    profiles->setAccount(account());
    profiles->getFollowers(account().did, 100, m_cursor);
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
        selector->setFollowing(m_followings);
        selector->setFollowers(m_followers);
        selector->setReady(true);
        FirehoseReceiver::getInstance()->start();
    }
}

void RealtimeFeedListModel::copyFollows(
        const QList<AtProtocolType::AppBskyActorDefs::ProfileView> &follows, bool is_following)
{
    for (const auto &follow : follows) {
        RealtimeFeed::UserInfo user;
        user.did = follow.did;
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

void RealtimeFeedListModel::getPosts()
{
    if (m_cueGetPost.isEmpty()) {
        m_runningCue = false;
        return;
    }

    // getPostsは最大25個までいっきに取得できる
    QStringList uris;
    for (int i = 0; i < 25; i++) {
        if (m_cueGetPost.isEmpty())
            break;
        if (m_cueGetPost.first().contains("/app.bsky.feed.post/")) {
            uris.append(m_cueGetPost.first());
        }
        m_cueGetPost.removeFirst();
    }
    if (uris.isEmpty()) {
        m_runningCue = false;
        return;
    } else if (uris.length() > 1) {
        qDebug() << "Many cue";
    }
    m_runningCue = true;

    AppBskyFeedGetPosts *posts = new AppBskyFeedGetPosts(this);
    connect(posts, &AppBskyFeedGetPosts::finished, [=](bool success) {
        if (success) {

            for (auto item = posts->postsList().cbegin(); item != posts->postsList().cend();
                 item++) {
                AtProtocolType::AppBskyFeedDefs::FeedViewPost view_post;
                view_post.post = *item;
                m_viewPostHash[item->cid] = view_post;

                bool visible = checkVisibility(item->cid);
                if (visible) {
                    beginInsertRows(QModelIndex(), 0, 0);
                    m_cidList.insert(0, item->cid);
                    endInsertRows();
                }
                m_originalCidList.insert(0, item->cid);

                // if (m_cidList.contains(item->cid)) {
                //     int r = m_cidList.indexOf(item->cid);
                //     if (r >= 0) {
                //         emit dataChanged(index(r), index(r));
                //     }
                // }
            }
        } else {
            emit errorOccured(posts->errorCode(), posts->errorMessage());
        }
        // 残ってたらもう1回
        QTimer::singleShot(0, this, &RealtimeFeedListModel::getPosts);
        posts->deleteLater();
    });
    posts->setAccount(account());
    posts->setLabelers(m_contentFilterLabels.labelerDids());
    posts->getPosts(uris);
}
