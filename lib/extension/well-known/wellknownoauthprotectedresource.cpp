#include "wellknownoauthprotectedresource.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

WellKnownOauthProtectedResource::WellKnownOauthProtectedResource(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void WellKnownOauthProtectedResource::resource()
{
    QUrlQuery url_query;

    get(QStringLiteral(".well-known/oauth-protected-resource"), url_query, false);
}

const AtProtocolType::WellKnownOauthProtectedResourceDefs::ResourceMetadata &
WellKnownOauthProtectedResource::resourceMetadata() const
{
    return m_resourceMetadata;
}

bool WellKnownOauthProtectedResource::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::WellKnownOauthProtectedResourceDefs::copyResourceMetadata(
                json_doc.object(), m_resourceMetadata);
    }

    return success;
}

}
