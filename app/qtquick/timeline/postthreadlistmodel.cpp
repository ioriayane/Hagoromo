#include "postthreadlistmodel.h"
#include "atprotocol/app/bsky/unspecced/appbskyunspeccedgetpostthreadv2.h"

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
        auto thread = new AtProtocolInterface::AppBskyUnspeccedGetPostThreadV2(this);
        connect(thread, &AtProtocolInterface::AppBskyUnspeccedGetPostThreadV2::finished,
                [=](bool success) {
                    if (success) {
                        copyFrom(thread->threadList());
                    } else {
                        emit errorOccurred(thread->errorCode(), thread->errorMessage());
                    }
                    QTimer::singleShot(100, this, &PostThreadListModel::displayQueuedPosts);
                    thread->deleteLater();
                });
        thread->setAccount(account());
        thread->setLabelers(labelerDids());
        thread->getPostThreadV2(postThreadUri(), true, 0, 0, QString());
    });
    return true;
}

void PostThreadListModel::finishedDisplayingQueuedPosts()
{
    setRunning(false);
}

void PostThreadListModel::copyFrom(
        const QList<AtProtocolType::AppBskyUnspeccedGetPostThreadV2::ThreadItem> &thread_list)
{
    QDateTime reference_time = QDateTime::currentDateTimeUtc();

    for (int i = 0; i < thread_list.length(); i++) {
        const auto &item = thread_list.at(i);
        if (item.value_type
            != AtProtocolType::AppBskyUnspeccedGetPostThreadV2::ThreadItemValueType::
                    value_AppBskyUnspeccedDefs_ThreadItemPost)
            continue;

        const AppBskyFeedDefs::PostView &post = item.value_AppBskyUnspeccedDefs_ThreadItemPost.post;

        if (item.depth == 0)
            m_postThreadCid = post.cid;

        PostCueItem cue_item;
        cue_item.cid = post.cid;
        cue_item.indexed_at = post.indexedAt;
        cue_item.reference_time = reference_time;
        m_cuePost.insert(0, cue_item);

        // Tokimekiの投票の取得のキューに入れる
        appendTokimekiPollToCue(post.cid, post.embed_AppBskyEmbedExternal_View);

        AppBskyFeedDefs::FeedViewPost feed_view_post;
        feed_view_post.post = post;
        feed_view_post.opThreadPostIndex =
                item.value_AppBskyUnspeccedDefs_ThreadItemPost.opThreadPostIndex;
        feed_view_post.opThreadPostCount =
                item.value_AppBskyUnspeccedDefs_ThreadItemPost.opThreadPostCount;
        m_viewPostHash[post.cid] = feed_view_post;

        // 前後のスレッド項目とのdepthの連続性から接続線の表示を判定する
        ThreadConnector connector;
        connector.top = (i != 0);
        connector.bottom =
                (i + 1 < thread_list.length() && thread_list.at(i + 1).depth == item.depth + 1);
        m_threadConnectorHash[post.cid] = connector;

        if (item.depth == 0) {
            // ラベラーの情報を取得してラベルの表示名を取得できるようにする
            QStringList labelers;
            for (const auto &label : post.author.labels) {
                if (!label.src.isEmpty() && !labelers.contains(label.src)) {
                    labelers.append(label.src);
                }
            }
            for (const auto &label : post.labels) {
                if (!label.src.isEmpty() && !labelers.contains(label.src)) {
                    labelers.append(label.src);
                }
            }
            if (!labelers.isEmpty()) {
                LabelProvider::getInstance()->update(labelers, account(), &m_labelConnector);
            }
        }
    }

    // Tokimekiの投票を取得
    getTokimekiPoll();
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
