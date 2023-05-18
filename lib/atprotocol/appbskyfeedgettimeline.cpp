#include "appbskyfeedgettimeline.h"
#include "../atprotocol/lexicons_func.h"

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

void AppBskyFeedGetTimeline::parseJson(const QString reply_json)
{
    bool success = false;
    m_feedList.clear();

    //    qDebug() << reply_json;

    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        qDebug() << "EMPTY";
        //    } else if (!json_doc.isArray()) {
        //        qDebug() << "NOT ARRAY";
    } else if (!json_doc.object().contains("feed")) {
        qDebug() << "Not found feed";
    } else {
        for (const auto &obj : json_doc.object().value("feed").toArray()) {
            AppBskyFeedDefs::FeedViewPost feed_item;

            AppBskyFeedDefs::copyFeedViewPost(obj.toObject(), feed_item);

            m_feedList.append(feed_item);
        }

        success = true;
    }

    emit finished(success);
}

}
