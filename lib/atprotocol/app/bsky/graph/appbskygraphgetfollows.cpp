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
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("actor"), actor);

    get(QStringLiteral("xrpc/app.bsky.graph.getFollows"), query);
}

const QList<AtProtocolType::AppBskyActorDefs::ProfileView> *
AppBskyGraphGetFollows::profileList() const
{
    return &m_profileList;
}

void AppBskyGraphGetFollows::parseJson(const QString reply_json)
{
    bool success = false;

    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
    } else if (!json_doc.object().contains(m_listKey)) {
    } else {
        for (const auto &obj : json_doc.object().value(m_listKey).toArray()) {
            AtProtocolType::AppBskyActorDefs::ProfileView profile;
            AtProtocolType::AppBskyActorDefs::copyProfileView(obj.toObject(), profile);

            m_profileList.append(profile);
        }
        success = true;
    }

    emit finished(success);
}

}
