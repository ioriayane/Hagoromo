#include "customfeedlistmodel.h"

#include "atprotocol/app/bsky/feed/appbskyfeedgetfeed.h"

#include <QPointer>

using AtProtocolInterface::AppBskyFeedGetFeed;

CustomFeedListModel::CustomFeedListModel(QObject *parent) : TimelineListModel { parent } { }

void CustomFeedListModel::getLatest()
{
    if (running())
        return;
    setRunning(true);

    QPointer<CustomFeedListModel> aliving(this);

    AppBskyFeedGetFeed *feed = new AppBskyFeedGetFeed();
    connect(feed, &AppBskyFeedGetFeed::finished, [=](bool success) {
        if (aliving) {
            if (success) {
                copyFrom(feed);
            } else {
                emit errorOccured(feed->errorMessage());
            }
            QTimer::singleShot(100, this, &CustomFeedListModel::displayQueuedPosts);
        }
        feed->deleteLater();
    });
    feed->setAccount(account());
    feed->getFeed(uri(), 50, QString());
}

QString CustomFeedListModel::uri() const
{
    return m_uri;
}

void CustomFeedListModel::setUri(const QString &newUri)
{
    if (m_uri == newUri)
        return;
    m_uri = newUri;
    emit uriChanged();
}
