#include "logfeedlistmodel.h"
#include "log/logmanager.h"
#include "atprotocol/lexicons.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

using namespace AtProtocolType;
using namespace AtProtocolInterface;

LogFeedListModel::LogFeedListModel(QObject *parent)
    : TimelineListModel { parent }, m_feedType(LogFeedListModelFeedType::DailyFeedType)
{
}

bool LogFeedListModel::getLatest()
{
    if (running() || did().isEmpty() || selectCondition().isEmpty()) {
        emit finished(false);
        return false;
    }
    setRunning(true);

    LogManager *manager = new LogManager(this);
    connect(manager, &LogManager::finishedSelection, this, [=](const QString &records) {
        if (!records.isEmpty()) {
            QJsonDocument doc = QJsonDocument::fromJson(records.toUtf8());
            QList<AtProtocolType::AppBskyFeedDefs::FeedViewPost> feed_view_post_list;

            // post処理ではcursorのみ処理する

            // for (const auto &src : doc.object().value("records").toArray()) {
            //     AppBskyFeedDefs::FeedViewPost view_post;
            //     view_post.post.uri = src.toObject().value("uri").toString();
            //     view_post.post.cid = src.toObject().value("cid").toString();
            //     LexiconsTypeUnknown::copyUnknown(src.toObject().value("value").toObject(),
            //                                      view_post.post.record);
            //     view_post.post.author.did = did();
            //     view_post.post.author.handle = handle();
            //     view_post.post.author.avatar = targetAvatar();
            //     view_post.post.indexedAt =
            //             src.toObject().value("value").toObject().value("createdAt").toString();
            //     feed_view_post_list.append(view_post);
            // }
            // copyFrom(feed_view_post_list);
        }
        // QTimer::singleShot(1, this, &LogFeedListModel::displayQueuedPosts);
        // deleteはDB更新後
        // manager->deleteLater();
    });
    connect(manager, &LogManager::finishedSelectionPosts, this, [=]() {
        if (!manager->feedViewPosts().isEmpty()) {
            copyFrom(manager->feedViewPosts());
        }
        QTimer::singleShot(1, this, &LogFeedListModel::displayQueuedPosts);
        manager->deleteLater();
    });
    manager->setAccount(account());
    emit manager->selectRecords(did(), static_cast<int>(feedType()), selectCondition(), QString(),
                                0);

    return true;
}

bool LogFeedListModel::getNext()
{
    //
    return true;
}

void LogFeedListModel::finishedDisplayingQueuedPosts()
{
    TimelineListModel::finishedDisplayingQueuedPosts();
    emit finished(true);
}

QString LogFeedListModel::selectCondition() const
{
    return m_selectCondition;
}

void LogFeedListModel::setSelectCondition(const QString &newSelectCondition)
{
    if (m_selectCondition == newSelectCondition)
        return;
    m_selectCondition = newSelectCondition;
    emit selectConditionChanged();
}

LogFeedListModel::LogFeedListModelFeedType LogFeedListModel::feedType() const
{
    return m_feedType;
}

void LogFeedListModel::setFeedType(LogFeedListModelFeedType newFeedType)
{
    if (m_feedType == newFeedType)
        return;
    m_feedType = newFeedType;
    emit feedTypeChanged();
}

QString LogFeedListModel::targetDid() const
{
    return account().did;
}

void LogFeedListModel::setTargetDid(const QString &newTargetDid)
{
    if (account().did == newTargetDid)
        return;
    setAccount(service(), newTargetDid, handle(), email(), accessJwt(), refreshJwt());
    emit targetDidChanged();
}

QString LogFeedListModel::targetHandle() const
{
    return account().handle;
}

void LogFeedListModel::setTargetHandle(const QString &newTargetHandle)
{
    if (account().handle == newTargetHandle)
        return;
    setAccount(service(), did(), newTargetHandle, email(), accessJwt(), refreshJwt());
    emit targetHandleChanged();
}

QString LogFeedListModel::targetAvatar() const
{
    return m_targetAvatar;
}

void LogFeedListModel::setTargetAvatar(const QString &newTargetAvatar)
{
    if (m_targetAvatar == newTargetAvatar)
        return;
    m_targetAvatar = newTargetAvatar;
    emit targetAvatarChanged();
}
