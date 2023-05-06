#include "accessatprotocol.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <qDebug>

AccessAtProtocol::AccessAtProtocol(QObject *parent) : QObject { parent }
{
    connect(&m_manager, &QNetworkAccessManager::finished, [=](QNetworkReply *reply) {
        qDebug() << "reply" << reply->error() << reply->url();

        if (reply->error() != QNetworkReply::NoError) {
            parseJson(QStringLiteral("{}"));
        } else {
            parseJson(QString::fromUtf8(reply->readAll()));
        }
    });
}

void AccessAtProtocol::setSession(const QString &did, const QString &handle, const QString &email,
                                  const QString &accessJwt, const QString &refresh_jwt)
{
    m_did = did;
    m_handle = handle;
    m_email = email;
    m_accessJwt = accessJwt;
    m_refreshJwt = refresh_jwt;
}

QString AccessAtProtocol::service() const
{
    return m_service;
}

void AccessAtProtocol::setService(const QString &newService)
{
    m_service = newService;
}

void AccessAtProtocol::get(const QString &endpoint, const QUrlQuery &query)
{
    QUrl url = QString("%1/%2").arg(service(), endpoint);
    url.setQuery(query);
    QNetworkRequest request(url);
    request.setRawHeader(QByteArray("Authorization"), QByteArray("Bearer ") + accessJwt().toUtf8());

    m_manager.get(request);
}

void AccessAtProtocol::post(const QString &endpoint, const QByteArray &json,
                            const bool with_auth_header)
{
    qDebug() << "AccessAtProtocol::post()" << this << endpoint << json;

    QNetworkRequest request(QUrl(QString("%1/%2").arg(service(), endpoint)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (with_auth_header) {
        request.setRawHeader(QByteArray("Authorization"),
                             QByteArray("Bearer ") + accessJwt().toUtf8());
    }

    m_manager.post(request, json);
}

void AccessAtProtocol::setRefreshJwt(const QString &newRefreshJwt)
{
    m_refreshJwt = newRefreshJwt;
}

void AccessAtProtocol::setAccessJwt(const QString &newAccessJwt)
{
    m_accessJwt = newAccessJwt;
}

void AccessAtProtocol::setEmail(const QString &newEmail)
{
    m_email = newEmail;
}

void AccessAtProtocol::setHandle(const QString &newHandle)
{
    m_handle = newHandle;
}

void AccessAtProtocol::setDid(const QString &newDid)
{
    m_did = newDid;
}

QString AccessAtProtocol::refreshJwt() const
{
    return m_refreshJwt;
}

QString AccessAtProtocol::accessJwt() const
{
    return m_accessJwt;
}

QString AccessAtProtocol::email() const
{
    return m_email;
}

QString AccessAtProtocol::handle() const
{
    return m_handle;
}

QString AccessAtProtocol::did() const
{
    return m_did;
}
