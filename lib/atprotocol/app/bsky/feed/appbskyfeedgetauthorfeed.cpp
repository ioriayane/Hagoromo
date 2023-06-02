#include "appbskyfeedgetauthorfeed.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyFeedGetAuthorFeed::AppBskyFeedGetAuthorFeed(QObject *parent)
    : AppBskyFeedGetTimeline { parent }
{
}

void AppBskyFeedGetAuthorFeed::getAuthorFeed(const QString &actor, const int limit,
                                             const QString &cursor)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("actor"), actor);

    get(QStringLiteral("xrpc/app.bsky.feed.getAuthorFeed"), query);
}

}
