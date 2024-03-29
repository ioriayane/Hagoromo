#include "appbskyactorgetprofiles.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyActorGetProfiles::AppBskyActorGetProfiles(QObject *parent) : AccessAtProtocol { parent } { }

void AppBskyActorGetProfiles::getProfiles(const QList<QString> &actors)
{
    QUrlQuery query;
    for (const auto &actor : actors) {
        query.addQueryItem(QStringLiteral("actors[]"), actor);
    }

    get(QStringLiteral("xrpc/app.bsky.actor.getProfiles"), query);
}

const QList<AtProtocolType::AppBskyActorDefs::ProfileViewDetailed> *
AppBskyActorGetProfiles::profileViewDetaileds() const
{
    return &m_profileViewDetaileds;
}

bool AppBskyActorGetProfiles::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains("profiles")) {
        success = false;
    } else {
        for (const auto &value : json_doc.object().value("profiles").toArray()) {
            AtProtocolType::AppBskyActorDefs::ProfileViewDetailed profile;
            AtProtocolType::AppBskyActorDefs::copyProfileViewDetailed(value.toObject(), profile);
            m_profileViewDetaileds.append(profile);
        }
    }

    return success;
}

}
