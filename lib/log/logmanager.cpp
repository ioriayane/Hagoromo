#include "logmanager.h"
#include "atprotocol/app/bsky/feed/appbskyfeedgetposts.h"
#include "atprotocol/com/atproto/sync/comatprotosyncgetrepo.h"
#include "atprotocol/lexicons_func.h"

#include <QDebug>
#include <QJsonDocument>
#include <QTimer>

using namespace AtProtocolType;
using AtProtocolInterface::AppBskyFeedGetPosts;
using AtProtocolInterface::ComAtprotoSyncGetRepo;

LogManager::LogManager(QObject *parent) : AtProtocolAccount { parent }
{
    m_logAccess.moveToThread(&m_thread);
    connect(this, &LogManager::updateDb, &m_logAccess, &LogAccess::updateDb);
    connect(this, &LogManager::dailyTotals, &m_logAccess, &LogAccess::dailyTotals);
    connect(this, &LogManager::monthlyTotals, &m_logAccess, &LogAccess::monthlyTotals);
    connect(this, &LogManager::statistics, &m_logAccess, &LogAccess::statistics);
    connect(this, &LogManager::selectRecords, &m_logAccess, &LogAccess::selectRecords);
    connect(&m_logAccess, &LogAccess::finishedUpdateDb, this,
            [=](bool success) { emit finished(success); });
    connect(&m_logAccess, &LogAccess::finishedTotals, this,
            [=](const QList<TotalItem> &list, const int max) { emit finishedTotals(list, max); });
    connect(&m_logAccess, &LogAccess::finishedSelection, this,
            [=](const QString &records, const QStringList &view_posts) {
                // view_postsには基本ポストのURIがくる
                // ただし既にDBに取得済みのViewPostが保存されているとJSONがくる
                emit finishedSelection(records);

                QJsonDocument json_records = QJsonDocument::fromJson(records.toUtf8());
                m_viewPosts.clear();
                m_cueGetPost.clear();
                m_postViews.clear();
                m_repostedPosts.clear();
                m_postViewsJson = QJsonArray();
                for (const auto &post_uri : view_posts) {
                    const QStringList post_items = post_uri.split("/");
                    if (post_uri.startsWith("at://")) {
                        if (post_items.contains("app.bsky.feed.post")) {
                            m_cueGetPost.append(post_uri);
                            m_viewPosts.append(post_uri); // getPostsした結果で置換する
                        } else if (post_items.contains("app.bsky.feed.repost")) {
                            for (const auto &record :
                                 json_records.object().value("records").toArray()) {
                                QString uri = record.toObject().value("uri").toString();
                                if (uri == post_uri) {
                                    // repostされた側のURIを取得
                                    QString reposted_uri = record.toObject()
                                                                   .value("value")
                                                                   .toObject()
                                                                   .value("subject")
                                                                   .toObject()
                                                                   .value("uri")
                                                                   .toString();
                                    if (reposted_uri.split("/").contains("app.bsky.feed.post")) {
                                        m_cueGetPost.append(reposted_uri);
                                        m_viewPosts.append(
                                                reposted_uri); // getPostsした結果で置換する
                                        m_repostedPosts[reposted_uri] = post_uri;
                                        break;
                                    }
                                }
                            }
                        }
                    } else {
                        m_viewPosts.append(post_uri);
                    }
                }
                getPosts();
            });
    connect(&m_logAccess, &LogAccess::finishedUpdateRecords, this,
            [=]() { emit finishedSelectionPosts(); });
    connect(&m_logAccess, &LogAccess::progressMessage, this,
            [=](const QString &message) { emit progressMessage(message); });
    m_thread.start();
}

LogManager::~LogManager()
{
    m_thread.exit();
    m_thread.wait();
}

void LogManager::update(const QString &service, const QString &did)
{
    qDebug() << "update" << service << did;

    if (service.isEmpty() || did.isEmpty()) {
        emit finished(false);
        return;
    }

    emit progressMessage("Downloading repository data ...");

    ComAtprotoSyncGetRepo *repo = new ComAtprotoSyncGetRepo(this);
    connect(repo, &ComAtprotoSyncGetRepo::finished, this, [=](bool success) {
        if (success) {
            emit updateDb(did, repo->repo());
        } else {
            emit errorOccured(repo->errorCode(), repo->errorMessage());
            emit finished(false);
        }
        repo->deleteLater();
    });
    repo->setService(service);
    repo->getRepo(did, QString());
}

void LogManager::clearDb(const QString &did)
{
    m_logAccess.removeDbFile(did);
}

const QList<AppBskyFeedDefs::FeedViewPost> &LogManager::feedViewPosts() const
{
    return m_feedViewPosts;
}

void LogManager::makeFeedViewPostList()
{
    QList<RecordPostItem> record_post_items;

    for (const auto &post_uri : std::as_const(m_viewPosts)) {
        if (post_uri.startsWith("at://")) {
            // APIで取得したデータを探して追加する
            for (const auto &post_view : std::as_const(m_postViews)) {
                if (post_view.uri == post_uri) {
                    AppBskyFeedDefs::FeedViewPost feed_view_post;
                    feed_view_post.post = post_view;
                    if (m_repostedPosts.contains(post_uri)) {
                        feed_view_post.reason_type =
                                AppBskyFeedDefs::FeedViewPostReasonType::reason_ReasonRepost;
                        feed_view_post.reason_ReasonRepost.by.displayName = this->displayName();
                        feed_view_post.reason_ReasonRepost.by.handle = this->handle();
                    }
                    m_feedViewPosts.append(feed_view_post);
                    break;
                }
            }
            // DBへのフィードバック用データ
            for (const auto &post_val : std::as_const(m_postViewsJson)) {
                if (post_val.toObject().value("uri").toString() == post_uri) {
                    RecordPostItem item;
                    item.uri = post_uri;
                    item.json = QJsonDocument(post_val.toObject()).toJson(QJsonDocument::Compact);
                    record_post_items.append(item);
                    break;
                }
            }
        } else {
            // dbに保存してあったpostのデータを追加する
            QJsonDocument doc = QJsonDocument::fromJson(post_uri.toUtf8());
            AppBskyFeedDefs::FeedViewPost feed_view_post;
            AppBskyFeedDefs::copyPostView(doc.object(), feed_view_post.post);
            if (m_repostedPosts.contains(post_uri)) {
                feed_view_post.reason_type =
                        AppBskyFeedDefs::FeedViewPostReasonType::reason_ReasonRepost;
                feed_view_post.reason_ReasonRepost.by.displayName = this->displayName();
                feed_view_post.reason_ReasonRepost.by.handle = this->handle();
            }
            m_feedViewPosts.append(feed_view_post);
        }
    }
    // record_post_itemsが0件でも必ず呼び出して更新完了シグナルを受ける
    emit m_logAccess.updateRecords(did(), record_post_items);
}

void LogManager::getPosts()
{
    if (m_cueGetPost.isEmpty()) {
        m_feedViewPosts.clear();
        makeFeedViewPostList();
        // 完了通知はDBの更新シグナルで実施
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

    AppBskyFeedGetPosts *posts = new AppBskyFeedGetPosts(this);
    connect(posts, &AppBskyFeedGetPosts::finished, [=](bool success) {
        if (success) {
            m_postViews.append(posts->postsList());

            QJsonDocument doc = QJsonDocument::fromJson(posts->replyJson().toUtf8());
            for (const auto &post_val : doc.object().value("posts").toArray()) {
                m_postViewsJson.append(post_val);
            }
        } else {
            emit errorOccured(posts->errorCode(), posts->errorMessage());
        }
        // 残ってたらもう1回
        QTimer::singleShot(10, this, &LogManager::getPosts);
        posts->deleteLater();
    });
    posts->setAccount(account());
    // posts->setLabelers(m_contentFilterLabels.labelerDids());
    posts->getPosts(uris);
}
