#include "anyfeedlistmodel.h"

#include "atprotocol/app/bsky/feed/appbskyfeedgetposts.h"
#include "atprotocol/com/atproto/repo/comatprotorepolistrecords.h"
#include "atprotocol/lexicons_func_unknown.h"

#include <QPointer>

using AtProtocolInterface::AppBskyFeedGetPosts;
using AtProtocolInterface::ComAtprotoRepoListRecords;

AnyFeedListModel::AnyFeedListModel(QObject *parent) : TimelineListModel { parent } { }

void AnyFeedListModel::getLatest()
{
    // 親クラスのm_cidListとm_viewPostHashは表示のために使う
    // listRecordで直接取得するレコード情報はこのクラスで別途保存する
    // 続きを読み込めるようにするときはこのクラスでカーソルの管理を含めて実施

    if (running())
        return;
    setRunning(true);

    QPointer<AnyFeedListModel> aliving(this);

    ComAtprotoRepoListRecords *records = new ComAtprotoRepoListRecords();
    connect(records, &ComAtprotoRepoListRecords::finished, [=](bool success) {
        if (aliving) {
            if (success) {
                for (const auto &record : *records->recordList()) {
                    m_recordHash[record.cid] = record;

                    QString cid;
                    switch (feedType()) {
                    case AnyFeedListModelFeedType::LikeFeedType:
                        cid = appendGetPostCue<AtProtocolType::AppBskyFeedLike::Main>(record.value);
                        break;
                    case AnyFeedListModelFeedType::RepostFeedType:
                        cid = appendGetPostCue<AtProtocolType::AppBskyFeedRepost::Main>(
                                record.value);
                        break;
                    default:
                        break;
                    }
                    if (!cid.isEmpty() && !m_cidList.contains(cid)) {
                        beginInsertRows(QModelIndex(), m_cidList.count(), m_cidList.count());
                        m_cidList.append(cid);
                        endInsertRows();
                    }
                }

                if (!m_cueGetPost.isEmpty()) {
                    QTimer::singleShot(100, this, &AnyFeedListModel::getPosts);
                }
            }
            setRunning(false);
        }
        records->deleteLater();
    });
    records->setAccount(account());
    switch (feedType()) {
    case AnyFeedListModelFeedType::LikeFeedType:
        records->listLikes(targetDid());
        break;
    case AnyFeedListModelFeedType::RepostFeedType:
        records->listReposts(targetDid());
        break;
    default:
        setRunning(false);
        delete records;
        break;
    }
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

void AnyFeedListModel::getPosts()
{
    if (m_cueGetPost.isEmpty())
        return;

    // getPostsは最大25個までいっきに取得できる
    QStringList uris;
    for (int i = 0; i < 25; i++) {
        if (m_cueGetPost.isEmpty())
            break;
        uris.append(m_cueGetPost.first());
        m_cueGetPost.removeFirst();
    }

    QPointer<AnyFeedListModel> aliving(this);

    AppBskyFeedGetPosts *posts = new AppBskyFeedGetPosts();
    connect(posts, &AppBskyFeedGetPosts::finished, [=](bool success) {
        if (aliving) {
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
                // 残ってたらもう1回
                if (!m_cueGetPost.isEmpty()) {
                    QTimer::singleShot(100, this, &AnyFeedListModel::getPosts);
                }
            }
        }
        posts->deleteLater();
    });
    posts->setAccount(account());
    posts->getPosts(uris);
}

template<typename T>
QString AnyFeedListModel::appendGetPostCue(const QVariant &record)
{
    QString cid;
    T data = AtProtocolType::LexiconsTypeUnknown::fromQVariant<T>(record);
    if (!data.subject.cid.isEmpty() && !m_cueGetPost.contains(data.subject.uri)) {
        m_cueGetPost.append(data.subject.uri);
        cid = data.subject.cid;
    }
    return cid;
}