#include "sitemochottaccountgetauthorization.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

SiteMochottAccountGetAuthorization::SiteMochottAccountGetAuthorization(QObject *parent)
    : AccessAtProtocol { parent }, m_authorized(false)
{
}

void SiteMochottAccountGetAuthorization::getAuthorization()
{
    QUrlQuery url_query;
    url_query.addQueryItem(QStringLiteral("display"), QStringLiteral("popup"));

    appendRawHeader(QStringLiteral("atproto-proxy"),
                    QStringLiteral("did:web:mochott.site#mochott_appview"));
    get(QStringLiteral("xrpc/site.mochott.account.getAuthorization"), url_query, true);
}

bool SiteMochottAccountGetAuthorization::authorized() const
{
    return m_authorized;
}

const QString &SiteMochottAccountGetAuthorization::authorizeUrl() const
{
    return m_authorizeUrl;
}

bool SiteMochottAccountGetAuthorization::parseJson(bool success, const QString reply_json)
{
    const QJsonObject json = QJsonDocument::fromJson(reply_json.toUtf8()).object();
    if (!json.value("authorized").isBool())
        return false;
    m_authorized = json.value("authorized").toBool();
    m_authorizeUrl = json.value("authorizeUrl").toString();
    return success;
}

}
