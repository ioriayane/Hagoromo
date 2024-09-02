#include "wellknownoauthauthorizationserver.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

WellKnownOauthAuthorizationServer::WellKnownOauthAuthorizationServer(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void WellKnownOauthAuthorizationServer::server()
{
    QUrlQuery url_query;

    get(QStringLiteral(".well-known/oauth-authorization-server"), url_query, false);
}

const AtProtocolType::WellKnownOauthAuthorizationServerDefs::ServerMetadata &
WellKnownOauthAuthorizationServer::serverMetadata() const
{
    return m_serverMetadata;
}

bool WellKnownOauthAuthorizationServer::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::WellKnownOauthAuthorizationServerDefs::copyServerMetadata(json_doc.object(),
                                                                                  m_serverMetadata);
    }

    return success;
}

}
