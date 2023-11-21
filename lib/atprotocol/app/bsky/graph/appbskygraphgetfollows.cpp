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

bool AppBskyGraphGetFollows::getFollows(const QString &actor, const int limit,
                                        const QString &cursor)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("actor"), actor);
    if (!cursor.isEmpty()) {
        query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    return get(QStringLiteral("xrpc/app.bsky.graph.getFollows"), query);
}

const QList<AtProtocolType::AppBskyActorDefs::ProfileView> *
AppBskyGraphGetFollows::profileList() const
{
    return &m_profileList;
}

bool AppBskyGraphGetFollows::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains(m_listKey)) {
        success = false;
    } else {
        setCursor(json_doc.object().value("cursor").toString());
        for (const auto &obj : json_doc.object().value(m_listKey).toArray()) {
            AtProtocolType::AppBskyActorDefs::ProfileView profile;
            AtProtocolType::AppBskyActorDefs::copyProfileView(obj.toObject(), profile);

            m_profileList.append(profile);
        }
    }

    return success;
}

}
