#include "appbskygraphgetfollows.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyGraphGetFollows::AppBskyGraphGetFollows(QObject *parent)
    : AccessAtProtocol { parent }, m_listKey("follows")
{
}

void AppBskyGraphGetFollows::getFollows(const QString &actor, const int limit,
                                        const QString &cursor)
{
    QUrlQuery url_query;
    if (!actor.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("actor"), actor);
    }
    if (limit > 0) {
        url_query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!cursor.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    get(QStringLiteral("xrpc/app.bsky.graph.getFollows"), url_query);
}

const AtProtocolType::AppBskyActorDefs::ProfileView &AppBskyGraphGetFollows::profileView() const
{
    return m_profileView;
}

const QList<AtProtocolType::AppBskyActorDefs::ProfileView> &
AppBskyGraphGetFollows::profileViewList() const
{
    return m_profileViewList;
}

bool AppBskyGraphGetFollows::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains(m_listKey)) {
        success = false;
    } else {
        setCursor(json_doc.object().value("cursor").toString());
        AtProtocolType::AppBskyActorDefs::copyProfileView(
                json_doc.object().value("subject").toObject(), m_profileView);
        for (const auto &value : json_doc.object().value(m_listKey).toArray()) {
            AtProtocolType::AppBskyActorDefs::ProfileView data;
            AtProtocolType::AppBskyActorDefs::copyProfileView(value.toObject(), data);
            m_profileViewList.append(data);
        }
    }

    return success;
}

}
