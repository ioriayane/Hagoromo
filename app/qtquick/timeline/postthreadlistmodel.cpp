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
                copyFrom(&thread->threadViewPost());
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

void PostThreadListModel::copyFrom(const AppBskyFeedDefs::ThreadViewPost *thread_view_post)
{
    if (thread_view_post == nullptr)
        return;

    const AppBskyFeedDefs::ThreadViewPost *origin = thread_view_post;
    const AppBskyFeedDefs::ThreadViewPost *current = thread_view_post;
    QList<const AppBskyFeedDefs::ThreadViewPost *> list;

    QDateTime reference_time = QDateTime::currentDateTimeUtc();
    while (current != nullptr
           && current->parent_type
                   == AppBskyFeedDefs::ThreadViewPostParentType::parent_ThreadViewPost) {
        current = current->parent_ThreadViewPost.get();
        if (current != nullptr) {
            list.push_front(current);
        } else {
            break;
        }
    }
    for (auto post : list) {
        // 親方向の投稿登録（2番目以降は必ず親がいる）
        copyFromMain(post, 1, reference_time, post != list.first());
    }

    copyFromMain(origin, 0, reference_time, !list.isEmpty());
}

// type
// 0 : 基準になっているポスト
// 1 : 親方向のポスト
// 2 : リプライ方向のポスト
void PostThreadListModel::copyFromMain(
        const AtProtocolType::AppBskyFeedDefs::ThreadViewPost *thread_view_post, const int type,
        QDateTime reference_time, bool has_parent)
{
    if (thread_view_post == nullptr)
        return;

    PostCueItem post;
    post.cid = thread_view_post->post.cid;
    post.indexed_at = thread_view_post->post.indexedAt;
    post.reference_time = reference_time;
    m_cuePost.insert(0, post);

    // Tokimekiの投票の取得のキューに入れる
    appendTokimekiPollToCue(thread_view_post->post.cid,
                            thread_view_post->post.embed_AppBskyEmbedExternal_View);

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
            copyFromMain(reply_view_post.get(), 2, reference_time, true);
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
        // Tokimekiの投票を取得
        getTokimekiPoll();
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
