#include "appbskyfeedgettimeline.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyFeedGetTimeline::AppBskyFeedGetTimeline(QObject *parent)
    : AccessAtProtocol { parent }, m_listKey("feed")
{
}

void AppBskyFeedGetTimeline::getTimeline(const QString &algorithm, const int limit,
                                         const QString &cursor)
{
    QUrlQuery url_query;
    if (!algorithm.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("algorithm"), algorithm);
    }
    if (limit > 0) {
        url_query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!cursor.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    get(QStringLiteral("xrpc/app.bsky.feed.getTimeline"), url_query);
}

const QList<AtProtocolType::AppBskyFeedDefs::FeedViewPost> &AppBskyFeedGetTimeline::feedList() const
{
    return m_feedList;
}

bool AppBskyFeedGetTimeline::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains(m_listKey)) {
        success = false;
    } else {
        setCursor(json_doc.object().value("cursor").toString());
        for (const auto &value : json_doc.object().value(m_listKey).toArray()) {
            AtProtocolType::AppBskyFeedDefs::FeedViewPost data;
            AtProtocolType::AppBskyFeedDefs::copyFeedViewPost(value.toObject(), data);
            m_feedList.append(data);
        }
    }

    return success;
}

}
