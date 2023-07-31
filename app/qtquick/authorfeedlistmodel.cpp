#include "authorfeedlistmodel.h"

#include "atprotocol/app/bsky/feed/appbskyfeedgetauthorfeed.h"

using AtProtocolInterface::AppBskyFeedGetAuthorFeed;

AuthorFeedListModel::AuthorFeedListModel(QObject *parent) : TimelineListModel { parent }
{
    setDisplayInterval(0);
}

void AuthorFeedListModel::getLatest()
{
    if (running())
        return;
    setRunning(true);

    updateContentFilterLabels([=]() {
        AppBskyFeedGetAuthorFeed *timeline = new AppBskyFeedGetAuthorFeed(this);
        connect(timeline, &AppBskyFeedGetAuthorFeed::finished, [=](bool success) {
            if (success) {
                copyFrom(timeline);
            } else {
                emit errorOccured(timeline->errorMessage());
            }
            QTimer::singleShot(100, this, &AuthorFeedListModel::displayQueuedPosts);
            timeline->deleteLater();
        });
        timeline->setAccount(account());
        timeline->getAuthorFeed(authorDid(), -1, QString());
    });
}

QString AuthorFeedListModel::authorDid() const
{
    return m_authorDid;
}

void AuthorFeedListModel::setAuthorDid(const QString &newAuthorDid)
{
    if (m_authorDid == newAuthorDid)
        return;
    m_authorDid = newAuthorDid;
    emit authorDidChanged();
}
