#include "realtimefeedlistmodel.h"

#include "atprotocol/app/bsky/feed/appbskyfeedgetposts.h"
#include "atprotocol/app/bsky/graph/appbskygraphgetfollows.h"
#include "atprotocol/app/bsky/graph/appbskygraphgetfollowers.h"
#include "atprotocol/app/bsky/feed/appbskyfeedgetpostthread.h"

using namespace RealtimeFeed;
using AtProtocolInterface::AppBskyFeedGetPosts;
using AtProtocolInterface::AppBskyFeedGetPostThread;
using AtProtocolInterface::AppBskyGraphGetFollowers;
using AtProtocolInterface::AppBskyGraphGetFollows;

RealtimeFeedListModel::RealtimeFeedListModel(QObject *parent)
    : TimelineListModel { parent }, m_receiving(false)
{
    FirehoseReceiver *receiver = FirehoseReceiver::getInstance();
    connect(receiver, &FirehoseReceiver::receivingChanged, this,
            &RealtimeFeedListModel::setReceiving);
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

        m_cueGetPostThread.append(selector->getOperationInfos(object));
        if (!m_cueGetPostThread.isEmpty()) {
            getPostThread();
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
        qDebug().noquote() << "Following count : " << m_followings.count();
        qDebug().noquote() << "Followers count : " << m_followers.count();
        selector->setFollowing(m_followings);
        selector->setFollowers(m_followers);
        selector->setReady(true);
#ifdef HAGOROMO_UNIT_TEST
        qDebug().noquote()
                << "FirehoseReceiver::getInstance()->start() --- No start on unit test mode";
#else
        FirehoseReceiver::getInstance()->start();
#endif
    }
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

void RealtimeFeedListModel::getPostThread()
{
    if (m_cueGetPostThread.isEmpty()) {
        m_runningCue = false;
        return;
    }

    RealtimeFeed::OperationInfo ope_info = m_cueGetPostThread.first();
    m_cueGetPostThread.removeFirst();

    m_runningCue = true;

    AppBskyFeedGetPostThread *post_thread = new AppBskyFeedGetPostThread(this);
    connect(post_thread, &AppBskyFeedGetPostThread::finished, [=](bool success) {
        if (success) {

            AtProtocolType::AppBskyFeedDefs::FeedViewPost view_post;
            view_post.post = post_thread->threadViewPost().post;
            if (ope_info.is_repost) {
                view_post.reason_type = AtProtocolType::AppBskyFeedDefs::FeedViewPostReasonType::
                        reason_ReasonRepost;
                view_post.reason_ReasonRepost.by.did = ope_info.reposted_by;
                view_post.reason_ReasonRepost.by.handle = ope_info.reposted_by_handle;
                view_post.reason_ReasonRepost.by.displayName = ope_info.reposted_by_display_name;
            }
            if (post_thread->threadViewPost().parent_type
                        == AtProtocolType::AppBskyFeedDefs::ThreadViewPostParentType::
                                parent_ThreadViewPost
                && post_thread->threadViewPost().parent_ThreadViewPost) {
                view_post.reply.parent_type =
                        AtProtocolType::AppBskyFeedDefs::ReplyRefParentType::parent_PostView;
                view_post.reply.parent_PostView =
                        post_thread->threadViewPost().parent_ThreadViewPost->post;
            }
            m_viewPostHash[view_post.post.cid] = view_post;
            bool visible = checkVisibility(view_post.post.cid);
            if (visible) {
                beginInsertRows(QModelIndex(), 0, 0);
                m_cidList.insert(0, view_post.post.cid);
                endInsertRows();
            }
            m_originalCidList.insert(0, view_post.post.cid);

        } else {
            emit errorOccured(post_thread->errorCode(), post_thread->errorMessage());
        }
        // 残ってたらもう1回
        QTimer::singleShot(0, this, &RealtimeFeedListModel::getPostThread);
        post_thread->deleteLater();
    });
    post_thread->setAccount(account());
    post_thread->setLabelers(labelerDids());
    post_thread->getPostThread(ope_info.uri, 0, 1);
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
