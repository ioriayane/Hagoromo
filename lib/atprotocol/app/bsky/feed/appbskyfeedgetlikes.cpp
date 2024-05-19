#include "appbskyfeedgetlikes.h"
#include "atprotocol/lexicons_func.h"
#include "atprotocol/lexicons_func_unknown.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyFeedGetLikes::AppBskyFeedGetLikes(QObject *parent) : AccessAtProtocol { parent } { }

void AppBskyFeedGetLikes::getLikes(const QString &uri, const QString &cid, const int limit,
                                   const QString &cursor)
{
    QUrlQuery url_query;
    if (!uri.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("uri"), uri);
    }
    if (!cid.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("cid"), cid);
    }
    if (limit > 0) {
        url_query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!cursor.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    get(QStringLiteral("xrpc/app.bsky.feed.getLikes"), url_query);
}

const QString &AppBskyFeedGetLikes::uri() const
{
    return m_uri;
}

const QString &AppBskyFeedGetLikes::cid() const
{
    return m_cid;
}

const QList<AtProtocolType::AppBskyFeedGetLikes::Like> &AppBskyFeedGetLikes::likesList() const
{
    return m_likesList;
}

bool AppBskyFeedGetLikes::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains("likes")) {
        success = false;
    } else {
        setCursor(json_doc.object().value("cursor").toString());
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("uri"), m_uri);
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("cid"), m_cid);
        for (const auto &value : json_doc.object().value("likes").toArray()) {
            AtProtocolType::AppBskyFeedGetLikes::Like data;
            AtProtocolType::AppBskyFeedGetLikes::copyLike(value.toObject(), data);
            m_likesList.append(data);
        }
    }

    return success;
}

}
