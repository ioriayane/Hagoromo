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
                                             const QString &cursor, const FilterType filter)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("actor"), actor);
    if (!cursor.isEmpty()) {
        query.addQueryItem(QStringLiteral("cursor"), cursor);
    }
    if (filter == FilterType::PostsNoReplies) {
        query.addQueryItem(QStringLiteral("filter"), "posts_no_replies");
    } else if (filter == FilterType::PostsWithMedia) {
        query.addQueryItem(QStringLiteral("filter"), "posts_with_media");
    } else {
        query.addQueryItem(QStringLiteral("filter"), "posts_with_replies");
    }

    get(QStringLiteral("xrpc/app.bsky.feed.getAuthorFeed"), query);
}

}
