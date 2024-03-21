#include "actorlikelistmodel.h"

#include "atprotocol/app/bsky/feed/appbskyfeedgetactorlikes.h"

using AtProtocolInterface::AppBskyFeedGetActorLikes;

ActorLikeListModel::ActorLikeListModel(QObject *parent) : TimelineListModel { parent } { }

bool ActorLikeListModel::getLatest()
{
    if (running() || actor().isEmpty())
        return false;
    setRunning(true);

    updateContentFilterLabels([=]() {
        AppBskyFeedGetActorLikes *likes = new AppBskyFeedGetActorLikes(this);
        connect(likes, &AppBskyFeedGetActorLikes::finished, [=](bool success) {
            if (success) {
                if (m_cidList.isEmpty() && m_cursor.isEmpty()) {
                    m_cursor = likes->cursor();
                }
                copyFrom(likes);
            } else {
                emit errorOccured(likes->errorCode(), likes->errorMessage());
            }
            QTimer::singleShot(100, this, &ActorLikeListModel::displayQueuedPosts);
            likes->deleteLater();
        });
        likes->setAccount(account());
        likes->setLabelers(m_contentFilterLabels.labelerDids());
        likes->getActorLikes(actor(), 0, QString());
    });
    return true;
}

bool ActorLikeListModel::getNext()
{
    if (running() || actor().isEmpty() || m_cursor.isEmpty())
        return false;
    setRunning(true);

    updateContentFilterLabels([=]() {
        AppBskyFeedGetActorLikes *likes = new AppBskyFeedGetActorLikes(this);
        connect(likes, &AppBskyFeedGetActorLikes::finished, [=](bool success) {
            if (success) {
                m_cursor = likes->cursor();
                copyFromNext(likes);
            } else {
                emit errorOccured(likes->errorCode(), likes->errorMessage());
            }
            QTimer::singleShot(10, this, &ActorLikeListModel::displayQueuedPostsNext);
            likes->deleteLater();
        });
        likes->setAccount(account());
        likes->setLabelers(m_contentFilterLabels.labelerDids());
        likes->getActorLikes(actor(), 0, m_cursor);
    });
    return true;
}

QString ActorLikeListModel::actor() const
{
    return m_actor;
}

void ActorLikeListModel::setActor(const QString &newActor)
{
    if (m_actor == newActor)
        return;
    m_actor = newActor;
    emit actorChanged();
}
