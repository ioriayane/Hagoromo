#include "authorfeedlistmodel.h"

#include "atprotocol/app/bsky/feed/appbskyfeedgetauthorfeed.h"

#include <QPointer>

using AtProtocolInterface::AppBskyFeedGetAuthorFeed;

AuthorFeedListModel::AuthorFeedListModel(QObject *parent) : TimelineListModel { parent } { }

void AuthorFeedListModel::getLatest()
{
    if (running())
        return;
    setRunning(true);

    QPointer<AuthorFeedListModel> aliving(this);

    AppBskyFeedGetAuthorFeed *timeline = new AppBskyFeedGetAuthorFeed();
    connect(timeline, &AppBskyFeedGetAuthorFeed::finished, [=](bool success) {
        if (aliving) {
            if (success) {
                copyFrom(timeline);
            }
            setRunning(false);
        }
        timeline->deleteLater();
    });
    timeline->setAccount(account());
    timeline->getAuthorFeed(authorDid(), -1, QString());
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
