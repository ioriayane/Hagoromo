#include "postthreadlistmodel.h"
#include "atprotocol/lexicons_func_unknown.h"
#include "atprotocol/app/bsky/feed/appbskyfeedgetpostthread.h"

using AtProtocolInterface::AppBskyFeedGetPostThread;
using namespace AtProtocolType;

PostThreadListModel::PostThreadListModel(QObject *parent) : TimelineListModel { parent } { }

void PostThreadListModel::getLatest()
{
    if (running() || postThreadUri().isEmpty())
        return;
    setRunning(true);

    updateContentFilterLabels([=]() {
        AppBskyFeedGetPostThread *thread = new AppBskyFeedGetPostThread(this);
        connect(thread, &AppBskyFeedGetPostThread::finished, [=](bool success) {
            if (success) {
                copyFrom(thread->threadViewPost(), true);
            } else {
                emit errorOccured(thread->errorCode(), thread->errorMessage());
            }
            QTimer::singleShot(100, this, &PostThreadListModel::displayQueuedPosts);
            thread->deleteLater();
        });
        thread->setAccount(account());
        thread->getPostThread(postThreadUri());
    });
}

void PostThreadListModel::finishedDisplayingQueuedPosts()
{
    setRunning(false);
}

void PostThreadListModel::copyFrom(const AppBskyFeedDefs::ThreadViewPost *thread_view_post,
                                   const bool is_basis)
{
    if (thread_view_post == nullptr)
        return;

    QDateTime reference_time = QDateTime::currentDateTimeUtc();
    bool has_parent = false;
    if (thread_view_post->parent_type
        == AppBskyFeedDefs::ThreadViewPostParentType::parent_ThreadViewPost) {
        copyFrom(thread_view_post->parent_ThreadViewPost.get());
        has_parent = true;
    }

    PostCueItem post;
    post.cid = thread_view_post->post.cid;
    post.indexed_at = thread_view_post->post.indexedAt;
    post.reference_time = reference_time;
    m_cuePost.insert(0, post);

    AppBskyFeedDefs::FeedViewPost feed_view_post;
    feed_view_post.post = thread_view_post->post;
    m_viewPostHash[thread_view_post->post.cid] = feed_view_post;

    ThreadConnector connector;
    connector.top = has_parent;
    if (!is_basis) {
        connector.bottom = true;
    } else {
        connector.bottom = (thread_view_post->replies_ThreadViewPost.length() > 0);
    }
    m_threadConnectorHash[thread_view_post->post.cid] = connector;

    // TODO
    // replies側の表示
    // こっちは枝分かれする場合があるので表示方法を検討すること
    // そもそも時系列が逆順になるので基準になっているpostを強調するなど工夫が必要
    // あと、repliesの1階層目しか表示できていない！
    for (const auto &view_post : thread_view_post->replies_ThreadViewPost) {
        AppBskyFeedDefs::FeedViewPost feed_view_post;
        feed_view_post.post = view_post->post;
        m_viewPostHash[view_post->post.cid] = feed_view_post;

        PostCueItem post;
        post.cid = view_post->post.cid;
        post.indexed_at = view_post->post.indexedAt;
        post.reference_time = reference_time;
        m_cuePost.insert(0, post);

        ThreadConnector connector;
        connector.top = true;
        m_threadConnectorHash[view_post->post.cid] = connector;
    }
}

QString PostThreadListModel::postThreadUri() const
{
    return m_postThreadUri;
}

void PostThreadListModel::setPostThreadUri(const QString &newPostThreadUri)
{
    if (m_postThreadUri == newPostThreadUri)
        return;
    m_postThreadUri = newPostThreadUri;
    emit postThreadUriChanged();
}
