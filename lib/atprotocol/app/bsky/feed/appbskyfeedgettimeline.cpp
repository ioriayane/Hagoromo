#include "appbskyfeedgettimeline.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>

using namespace AtProtocolType;

namespace AtProtocolInterface {

AppBskyFeedGetTimeline::AppBskyFeedGetTimeline(QObject *parent) : AccessAtProtocol { parent } { }

void AppBskyFeedGetTimeline::getTimeline()
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("actor"), handle());
    //    query.addQueryItem(QStringLiteral("actor"), cursor);

    get(QStringLiteral("xrpc/app.bsky.feed.getTimeline"), query);
}

const QList<AppBskyFeedDefs::FeedViewPost> *AppBskyFeedGetTimeline::feedList() const
{
    return &m_feedList;
}

void AppBskyFeedGetTimeline::parseJson(bool success, const QString reply_json)
{
    m_feedList.clear();

    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains("feed")) {
        success = false;
    } else {
        for (const auto &obj : json_doc.object().value("feed").toArray()) {
            AppBskyFeedDefs::FeedViewPost feed_item;

            AppBskyFeedDefs::copyFeedViewPost(obj.toObject(), feed_item);

            m_feedList.append(feed_item);
        }
    }

    emit finished(success);
}

}
