#include "anyfeedlistmodel.h"

#include "atprotocol/app/bsky/feed/appbskyfeedgetposts.h"
#include "atprotocol/com/atproto/repo/comatprotorepolistrecords.h"
#include "atprotocol/lexicons_func_unknown.h"

using AtProtocolInterface::AppBskyFeedGetPosts;
using AtProtocolInterface::ComAtprotoRepoListRecords;

AnyFeedListModel::AnyFeedListModel(QObject *parent) : TimelineListModel { parent }
{
    setDisplayInterval(0);
}

bool AnyFeedListModel::getLatest()
{
    // 親クラスのm_cidListとm_viewPostHashは表示のために使う
    // listRecordで直接取得するレコード情報はこのクラスで別途保存する
    // 続きを読み込めるようにするときはこのクラスでカーソルの管理を含めて実施

    if (running())
        return false;
    setRunning(true);

    updateContentFilterLabels([=]() {
        ComAtprotoRepoListRecords *records = new ComAtprotoRepoListRecords(this);
        connect(records, &ComAtprotoRepoListRecords::finished, [=](bool success) {
            if (success) {
                QDateTime reference_time = QDateTime::currentDateTimeUtc();

                if (m_cidList.isEmpty() && m_cursor.isEmpty()) {
                    m_cursor = records->cursor();
                }
                for (const auto &record : *records->recordList()) {
                    m_recordHash[record.cid] = record;

                    QString cid;
                    QString indexed_at;
                    switch (feedType()) {
                    case AnyFeedListModelFeedType::LikeFeedType:
                        cid = appendGetPostCue<AtProtocolType::AppBskyFeedLike::Main>(record.value);
                        indexed_at =
                                getIndexedAt<AtProtocolType::AppBskyFeedLike::Main>(record.value);
                        break;
                    case AnyFeedListModelFeedType::RepostFeedType:
                        cid = appendGetPostCue<AtProtocolType::AppBskyFeedRepost::Main>(
                                record.value);
                        indexed_at =
                                getIndexedAt<AtProtocolType::AppBskyFeedRepost::Main>(record.value);
                        break;
                    default:
                        break;
                    }
                    if (!cid.isEmpty() && !m_cidList.contains(cid)) {
                        PostCueItem post;
                        post.cid = cid;
                        post.indexed_at = indexed_at;
                        post.reference_time = reference_time;
                        m_cuePost.insert(0, post);
                    }
                }
            } else {
                emit errorOccured(records->errorCode(), records->errorMessage());
            }
            QTimer::singleShot(100, this, &AnyFeedListModel::displayQueuedPosts);
            records->deleteLater();
        });
        records->setAccount(account());
        switch (feedType()) {
        case AnyFeedListModelFeedType::LikeFeedType:
            records->listLikes(targetDid(), QString());
            break;
        case AnyFeedListModelFeedType::RepostFeedType:
            records->listReposts(targetDid(), QString());
            break;
        default:
            setRunning(false);
            delete records;
            break;
        }
    });
    return true;
}

bool AnyFeedListModel::getNext()
{
    if (running() || m_cursor.isEmpty())
        return false;
    setRunning(true);

    updateContentFilterLabels([=]() {
        ComAtprotoRepoListRecords *records = new ComAtprotoRepoListRecords(this);
        connect(records, &ComAtprotoRepoListRecords::finished, [=](bool success) {
            if (success) {
                QDateTime reference_time = QDateTime::currentDateTimeUtc();

                m_cursor = records->cursor();

                for (const auto &record : *records->recordList()) {
                    m_recordHash[record.cid] = record;

                    QString cid;
                    QString indexed_at;
                    switch (feedType()) {
                    case AnyFeedListModelFeedType::LikeFeedType:
                        cid = appendGetPostCue<AtProtocolType::AppBskyFeedLike::Main>(record.value);
                        indexed_at =
                                getIndexedAt<AtProtocolType::AppBskyFeedLike::Main>(record.value);
                        break;
                    case AnyFeedListModelFeedType::RepostFeedType:
                        cid = appendGetPostCue<AtProtocolType::AppBskyFeedRepost::Main>(
                                record.value);
                        indexed_at =
                                getIndexedAt<AtProtocolType::AppBskyFeedRepost::Main>(record.value);
                        break;
                    default:
                        break;
                    }
                    if (!cid.isEmpty() && !m_cidList.contains(cid)) {
                        PostCueItem post;
                        post.cid = cid;
                        post.indexed_at = indexed_at;
                        post.reference_time = reference_time;
                        m_cuePost.insert(0, post);
                    }
                }
            } else {
                emit errorOccured(records->errorCode(), records->errorMessage());
            }
            QTimer::singleShot(100, this, &AnyFeedListModel::displayQueuedPostsNext);
            records->deleteLater();
        });
        records->setAccount(account());
        switch (feedType()) {
        case AnyFeedListModelFeedType::LikeFeedType:
            records->listLikes(targetDid(), m_cursor);
            break;
        case AnyFeedListModelFeedType::RepostFeedType:
            records->listReposts(targetDid(), m_cursor);
            break;
        default:
            setRunning(false);
            delete records;
            break;
        }
    });
    return true;
}

QString AnyFeedListModel::targetDid() const
{
    return m_targetDid;
}

void AnyFeedListModel::setTargetDid(const QString &newTargetDid)
{
    if (m_targetDid == newTargetDid)
        return;
    m_targetDid = newTargetDid;
    emit targetDidChanged();
}

AnyFeedListModel::AnyFeedListModelFeedType AnyFeedListModel::feedType() const
{
    return m_feedType;
}

void AnyFeedListModel::setFeedType(AnyFeedListModelFeedType newFeedType)
{
    if (m_feedType == newFeedType)
        return;
    m_feedType = newFeedType;
    emit feedTypeChanged();
}

void AnyFeedListModel::finishedDisplayingQueuedPosts()
{
    QTimer::singleShot(0, this, &AnyFeedListModel::getPosts);
}

void AnyFeedListModel::getPosts()
{
    if (m_cueGetPost.isEmpty()) {
        setRunning(false);
        return;
    }

    // getPostsは最大25個までいっきに取得できる
    QStringList uris;
    for (int i = 0; i < 25; i++) {
        if (m_cueGetPost.isEmpty())
            break;
        uris.append(m_cueGetPost.first());
        m_cueGetPost.removeFirst();
    }

    AppBskyFeedGetPosts *posts = new AppBskyFeedGetPosts(this);
    connect(posts, &AppBskyFeedGetPosts::finished, [=](bool success) {
        if (success) {
            QStringList new_cid;

            for (auto item = posts->postList()->crbegin(); item != posts->postList()->crend();
                 item++) {
                AtProtocolType::AppBskyFeedDefs::FeedViewPost view_post;
                view_post.post = *item;
                m_viewPostHash[item->cid] = view_post;

                if (m_cidList.contains(item->cid)) {
                    int r = m_cidList.indexOf(item->cid);
                    if (r >= 0) {
                        emit dataChanged(index(r), index(r));
                    }
                }
            }
        } else {
            emit errorOccured(posts->errorCode(), posts->errorMessage());
        }
        // 残ってたらもう1回
        QTimer::singleShot(100, this, &AnyFeedListModel::getPosts);
        posts->deleteLater();
    });
    posts->setAccount(account());
    posts->setLabelers(m_contentFilterLabels.labelerDids());
    posts->getPosts(uris);
}

template<typename T>
QString AnyFeedListModel::appendGetPostCue(const QVariant &record)
{
    QString cid;
    T data = AtProtocolType::LexiconsTypeUnknown::fromQVariant<T>(record);
    if (!data.subject.cid.isEmpty() && !m_cueGetPost.contains(data.subject.uri)) {
        if (!data.subject.uri.contains("/app.bsky.feed.generator/")) {
            m_cueGetPost.append(data.subject.uri);
            cid = data.subject.cid;
        }
    }
    return cid;
}

template<typename T>
QString AnyFeedListModel::getIndexedAt(const QVariant &record)
{
    T data = AtProtocolType::LexiconsTypeUnknown::fromQVariant<T>(record);
    return data.createdAt;
}
