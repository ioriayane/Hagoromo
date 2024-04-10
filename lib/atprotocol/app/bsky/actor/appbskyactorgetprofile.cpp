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
    if (!actor.isEmpty()) {
        query.addQueryItem(QStringLiteral("actor"), actor);
    }

    get(QStringLiteral("xrpc/app.bsky.actor.getProfile"), query);
}

const AtProtocolType::AppBskyActorDefs::ProfileViewDetailed &
AppBskyActorGetProfile::profileViewDetailed() const
{
    return m_profileViewDetailed;
}

bool AppBskyActorGetProfile::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::AppBskyActorDefs::copyProfileViewDetailed(json_doc.object(),
                                                                  m_profileViewDetailed);
    }

    return success;
}

}
