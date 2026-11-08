#include "sitemochottpublicationgetpublication.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

SiteMochottPublicationGetPublication::SiteMochottPublicationGetPublication(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void SiteMochottPublicationGetPublication::getPublication(const QString &did)
{
    QUrlQuery url_query;
    url_query.addQueryItem(QStringLiteral("did"), did);

    get(QStringLiteral("xrpc/site.mochott.publication.getPublication"), url_query, false);
}

const QString &SiteMochottPublicationGetPublication::uri() const
{
    return m_uri;
}

const QString &SiteMochottPublicationGetPublication::standardPublication() const
{
    return m_standardPublication;
}

bool SiteMochottPublicationGetPublication::parseJson(bool success, const QString reply_json)
{
    const QJsonObject publication =
            QJsonDocument::fromJson(reply_json.toUtf8()).object().value("publication").toObject();
    m_uri = publication.value("uri").toString();
    m_standardPublication = publication.value("standardPublication").toString();
    return success && !m_uri.isEmpty();
}

}
