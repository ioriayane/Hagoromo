#include "appbskyfeedgetrepostedby.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyFeedGetRepostedBy::AppBskyFeedGetRepostedBy(QObject *parent)
    : AccessAtProtocol { parent } { }

const QList<AtProtocolType::AppBskyActorDefs::ProfileView> *
AppBskyFeedGetRepostedBy::profileViewList() const
{
    return &m_profileViewList;
}

void AppBskyFeedGetRepostedBy::getRepostedBy(const QString &uri, const QString &cid,
                                             const int limit, const QString &cursor)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("uri"), uri);
    if (limit > 0) {
        query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!cursor.isEmpty()) {
        query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    get(QStringLiteral("xrpc/app.bsky.feed.getRepostedBy"), query);
}

void AppBskyFeedGetRepostedBy::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        setCursor(json_doc.object().value("cursor").toString());
        for (const auto &obj : json_doc.object().value("repostedBy").toArray()) {
            AtProtocolType::AppBskyActorDefs::ProfileView profile;
            AtProtocolType::AppBskyActorDefs::copyProfileView(obj.toObject(), profile);
            m_profileViewList.append(profile);
        }
    }

    emit finished(success);
}

}
