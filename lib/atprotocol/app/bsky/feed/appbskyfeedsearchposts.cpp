#include "appbskyfeedsearchposts.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

using namespace AtProtocolType;

namespace AtProtocolInterface {

AppBskyFeedSearchPosts::AppBskyFeedSearchPosts(QObject *parent) : AppBskyFeedGetTimeline { parent }
{
}

void AppBskyFeedSearchPosts::searchPosts(const QString &q, const int limit, const QString &cursor)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("q"), q);
    if (limit > 0) {
        query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!cursor.isEmpty()) {
        query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    get(QStringLiteral("xrpc/app.bsky.feed.searchPosts"), query);
}

bool AppBskyFeedSearchPosts::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        setCursor(json_doc.object().value("cursor").toString());
        for (const auto &obj : json_doc.object().value("posts").toArray()) {
            AtProtocolType::AppBskyFeedDefs::FeedViewPost feed;

            AppBskyFeedDefs::copyPostView(obj.toObject(), feed.post);
            m_feedList.append(feed);
        }
    }

    return success;
}

}
