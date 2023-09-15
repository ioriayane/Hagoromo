#include "accessatprotocol.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QDebug>
#include <QFileInfo>
#include <QHttpMultiPart>
#include <QDateTime>
#include <QUrlQuery>

#define LOG_DATETIME QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss")

namespace AtProtocolInterface {

AccessAtProtocol::AccessAtProtocol(QObject *parent) : QObject { parent }
{
    connect(&m_manager, &QNetworkAccessManager::finished, [=](QNetworkReply *reply) {
        qDebug() << LOG_DATETIME << reply->error() << reply->url();
        m_replyJson = QString::fromUtf8(reply->readAll());
        m_errorCode.clear();
        m_errorMessage.clear();

        if (reply->error() != QNetworkReply::NoError) {
            qCritical() << LOG_DATETIME << m_replyJson;
            parseErrorJson(m_replyJson);
            emit finished(false);
        } else {
            parseJson(true, m_replyJson);
        }

        reply->deleteLater();
    });
}

const AccountData &AccessAtProtocol::account() const
{
    return m_account;
}

void AccessAtProtocol::setAccount(const AccountData &account)
{
    m_account.service = account.service;
    m_account.identifier.clear();
    m_account.password.clear();

    m_account.did = account.did;
    m_account.handle = account.handle;
    m_account.email = account.email;
    m_account.accessJwt = account.accessJwt;
    m_account.refreshJwt = account.refreshJwt;
    m_account.status = account.status;
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

void AccessAtProtocol::get(const QString &endpoint, const QUrlQuery &query,
                           const bool with_auth_header)
{
    if (accessJwt().isEmpty() && with_auth_header) {
        qCritical() << LOG_DATETIME << "AccessAtProtocol::get()"
                    << "Emty accessJwt!";
        return;
    }

    qDebug() << LOG_DATETIME << "AccessAtProtocol::get()" << this;
    qDebug().noquote() << "   " << handle();
    qDebug().noquote() << "   " << endpoint;
    qDebug().noquote() << "   " << query.toString();

    QUrl url = QString("%1/%2").arg(service(), endpoint);
    url.setQuery(query);
    QNetworkRequest request(url);
    if (with_auth_header) {
        request.setRawHeader(QByteArray("Authorization"),
                             QByteArray("Bearer ") + accessJwt().toUtf8());
    }

    m_manager.get(request);
}

void AccessAtProtocol::post(const QString &endpoint, const QByteArray &json,
                            const bool with_auth_header)
{
    qDebug() << LOG_DATETIME << "AccessAtProtocol::post()" << this;
    qDebug().noquote() << "   " << handle();
    qDebug().noquote() << "   " << endpoint;
    qDebug().noquote() << "   " << json;

    QNetworkRequest request(QUrl(QString("%1/%2").arg(service(), endpoint)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (with_auth_header) {
        if (accessJwt().isEmpty()) {
            qCritical() << LOG_DATETIME << "AccessAtProtocol::post()"
                        << "Empty accessJwt!";
            return;
        }

        request.setRawHeader(QByteArray("Authorization"),
                             QByteArray("Bearer ") + accessJwt().toUtf8());
    }

    m_manager.post(request, json);
}

void AccessAtProtocol::postWithImage(const QString &endpoint, const QString &path)
{
    if (accessJwt().isEmpty()) {
        qCritical() << LOG_DATETIME << "AccessAtProtocol::postWithImage()"
                    << "Empty accessJwt!";
        return;
    }
    if (!QFile::exists(path)) {
        qCritical() << LOG_DATETIME << "AccessAtProtocol::postWithImage()"
                    << "Not found" << path;
        return;
    }
    qDebug() << LOG_DATETIME << "AccessAtProtocol::postWithImage()" << this << endpoint << path;

    QFileInfo info(path);
    QNetworkRequest request(QUrl(QString("%1/%2").arg(service(), endpoint)));
    request.setRawHeader(QByteArray("Authorization"), QByteArray("Bearer ") + accessJwt().toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QString("image/%1").arg(info.suffix().toLower()));

    QFile *file = new QFile(path);
    if (!file->open(QIODevice::ReadOnly)) {
        qCritical() << LOG_DATETIME << "AccessAtProtocol::postWithImage()"
                    << "Not open" << path;
        delete file;
        return;
    }

    QNetworkReply *reply = m_manager.post(request, file);
    file->setParent(reply);
}

void AccessAtProtocol::parseErrorJson(const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.object().contains("error") && json_doc.object().contains("error")) {
        m_errorCode = json_doc.object().value("error").toString();
        m_errorMessage = json_doc.object().value("message").toString();
    } else {
        m_errorCode = QStringLiteral("Unknown");
        m_errorMessage = m_replyJson;
    }
}

QString AccessAtProtocol::cursor() const
{
    return m_cursor;
}

void AccessAtProtocol::setCursor(const QString &newCursor)
{
    m_cursor = newCursor;
}

QString AccessAtProtocol::errorMessage() const
{
    return m_errorMessage;
}

QString AccessAtProtocol::replyJson() const
{
    return m_replyJson;
}

QString AccessAtProtocol::errorCode() const
{
    return m_errorCode;
}

}
