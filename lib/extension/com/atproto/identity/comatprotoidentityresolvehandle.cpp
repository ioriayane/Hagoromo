#include "comatprotoidentityresolvehandle.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

ComAtprotoIdentityResolveHandle::ComAtprotoIdentityResolveHandle(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ComAtprotoIdentityResolveHandle::resolveHandle(const QString &handle)
{
    QUrlQuery url_query;
    url_query.addQueryItem(QStringLiteral("handle"), handle);

    get(QStringLiteral("xrpc/com.atproto.identity.resolveHandle"), url_query, false);
}

QString ComAtprotoIdentityResolveHandle::did() const
{
    return m_did;
}

bool ComAtprotoIdentityResolveHandle::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    m_did = json_doc.object().value("did").toString();
    if (!m_did.startsWith("did:")) {
        success = false;
    }
    return success;
}

}
