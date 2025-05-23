#include "postthreadlistmodel.h"
#include "atprotocol/app/bsky/feed/appbskyfeedgetpostthread.h"

using AtProtocolInterface::AppBskyFeedGetPostThread;
using namespace AtProtocolType;

PostThreadListModel::PostThreadListModel(QObject *parent)
    : TimelineListModel { parent }, m_labelConnector(this)
{
    connect(&m_labelConnector, &LabelConnector::finished, this,
            &PostThreadListModel::finishedConnector);
}

PostThreadListModel::~PostThreadListModel()
{
    disconnect(&m_labelConnector, &LabelConnector::finished, this,
               &PostThreadListModel::finishedConnector);
}

bool PostThreadListModel::getLatest()
{
    if (running() || postThreadUri().isEmpty())
        return false;
    setRunning(true);

    m_postThreadCid.clear();
    updateContentFilterLabels([=]() {
        AppBskyFeedGetPostThread *thread = new AppBskyFeedGetPostThread(this);
        connect(thread, &AppBskyFeedGetPostThread::finished, [=](bool success) {
            if (success) {
                m_postThreadCid = thread->threadViewPost().post.cid;
                copyFrom(&thread->threadViewPost(), 0);
            } else {
                emit errorOccured(thread->errorCode(), thread->errorMessage());
            }
            QTimer::singleShot(100, this, &PostThreadListModel::displayQueuedPosts);
            thread->deleteLater();
        });
        thread->setAccount(account());
        thread->setLabelers(labelerDids());
        thread->getPostThread(postThreadUri(), 0, 0);
    });
    return true;
}

void PostThreadListModel::finishedDisplayingQueuedPosts()
{
    setRunning(false);
}

// type
// 0 : 基準になっているポスト
// 1 : 親方向のポスト
// 2 : リプライ方向のポスト
void PostThreadListModel::copyFrom(const AppBskyFeedDefs::ThreadViewPost *thread_view_post,
                                   const int type)
{
    if (thread_view_post == nullptr)
        return;

    QDateTime reference_time = QDateTime::currentDateTimeUtc();
    bool has_parent = false;
    if (thread_view_post->parent_type
        == AppBskyFeedDefs::ThreadViewPostParentType::parent_ThreadViewPost) {
        copyFrom(thread_view_post->parent_ThreadViewPost.get(), 1);
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
    if (type == 0) {
        connector.top = has_parent;
        connector.bottom = (thread_view_post->replies_ThreadViewPost.length() > 0);
    } else if (type == 1) {
        connector.top = has_parent;
        connector.bottom = true;
    } else if (type == 2) {
        connector.top = true;
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
        connector.bottom = !view_post->replies_ThreadViewPost.isEmpty();
        m_threadConnectorHash[view_post->post.cid] = connector;

        for (const auto &reply_view_post : view_post->replies_ThreadViewPost) {
            copyFrom(reply_view_post.get(), 2);
        }
    }

    if (type == 0) {
        // ラベラーの情報を取得してラベルの表示名を取得できるようにする
        QStringList labelers;
        for (const auto &label : thread_view_post->post.author.labels) {
            if (!label.src.isEmpty() && !labelers.contains(label.src)) {
                labelers.append(label.src);
            }
        }
        for (const auto &label : thread_view_post->post.labels) {
            if (!label.src.isEmpty() && !labelers.contains(label.src)) {
                labelers.append(label.src);
            }
        }
        if (!labelers.isEmpty()) {
            LabelProvider::getInstance()->update(labelers, account(), &m_labelConnector);
        }
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

void PostThreadListModel::finishedConnector(const QString &labeler_did)
{
    Q_UNUSED(labeler_did)
    int row = m_cidList.indexOf(m_postThreadCid);
    if (row < 0)
        return;

    emit dataChanged(index(row), index(row),
                     QVector<int>() << LabelsRole << LabelIconsRole << AuthorLabelsRole
                                    << AuthorLabelIconsRole);
}
