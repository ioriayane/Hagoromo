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
    m_account.did = did;
    m_account.handle = handle;
    m_account.email = email;
    m_account.accessJwt = accessJwt;
    m_account.refreshJwt = refresh_jwt;
}

QString AccessAtProtocol::service() const
{
    return m_account.service;
}

void AccessAtProtocol::setService(const QString &newService)
{
    m_account.service = newService;
}

QString AccessAtProtocol::did() const
{
    return m_account.did;
}

QString AccessAtProtocol::handle() const
{
    return m_account.handle;
}

QString AccessAtProtocol::email() const
{
    return m_account.email;
}

QString AccessAtProtocol::accessJwt() const
{
    return m_account.accessJwt;
}

QString AccessAtProtocol::refreshJwt() const
{
    return m_account.refreshJwt;
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
