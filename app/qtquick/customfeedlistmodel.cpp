#include "customfeedlistmodel.h"

#include "atprotocol/app/bsky/feed/appbskyfeedgetfeed.h"

using AtProtocolInterface::AppBskyFeedGetFeed;

CustomFeedListModel::CustomFeedListModel(QObject *parent) : TimelineListModel { parent } { }

void CustomFeedListModel::getLatest()
{
    if (running())
        return;
    setRunning(true);

    updateContentFilterLabels([=]() {
        AppBskyFeedGetFeed *feed = new AppBskyFeedGetFeed(this);
        connect(feed, &AppBskyFeedGetFeed::finished, [=](bool success) {
            if (success) {
                copyFrom(feed);
            } else {
                emit errorOccured(feed->errorMessage());
            }
            QTimer::singleShot(100, this, &CustomFeedListModel::displayQueuedPosts);
            feed->deleteLater();
        });
        feed->setAccount(account());
        feed->getFeed(uri(), 50, QString());
    });
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
