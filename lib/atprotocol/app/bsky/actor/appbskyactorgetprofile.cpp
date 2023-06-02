#include "appbskyactorgetprofile.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyActorGetProfile::AppBskyActorGetProfile(QObject *parent) : AccessAtProtocol { parent } { }

void AppBskyActorGetProfile::getProfile(const QString &actor)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("actor"), actor);

    get(QStringLiteral("xrpc/app.bsky.actor.getProfile"), query);
}

void AppBskyActorGetProfile::parseJson(const QString reply_json)
{
    bool success = false;

    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
    } else {
        AtProtocolType::AppBskyActorDefs::copyProfileViewDetailed(json_doc.object(),
                                                                  m_profileViewDetailed);
        success = true;
    }

    emit finished(success);
}

AtProtocolType::AppBskyActorDefs::ProfileViewDetailed
AppBskyActorGetProfile::profileViewDetailed() const
{
    return m_profileViewDetailed;
}

}
