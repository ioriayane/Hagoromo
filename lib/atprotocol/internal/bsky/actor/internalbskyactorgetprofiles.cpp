#include "internalbskyactorgetprofiles.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

InternalBskyActorGetProfiles::InternalBskyActorGetProfiles(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void InternalBskyActorGetProfiles::getProfiles(const QList<QString> &dids, const QString &viewer,
                                               const QList<QString> &socialProof)
{
    QUrlQuery url_query;
    for (const auto &value : dids) {
        url_query.addQueryItem(QStringLiteral("dids"), value);
    }
    if (!viewer.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("viewer"), viewer);
    }
    for (const auto &value : socialProof) {
        url_query.addQueryItem(QStringLiteral("socialProof"), value);
    }

    get(QStringLiteral("xrpc/internal.bsky.actor.getProfiles"), url_query);
}

const QList<AtProtocolType::AppBskyActorDefs::ProfileViewDetailed> &
InternalBskyActorGetProfiles::profilesList() const
{
    return m_profilesList;
}

bool InternalBskyActorGetProfiles::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains("profiles")) {
        success = false;
    } else {
        for (const auto &value : json_doc.object().value("profiles").toArray()) {
            AtProtocolType::AppBskyActorDefs::ProfileViewDetailed data;
            AtProtocolType::AppBskyActorDefs::copyProfileViewDetailed(value.toObject(), data);
            m_profilesList.append(data);
        }
    }

    return success;
}

}
