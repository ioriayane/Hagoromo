#include "accessatprotocol.h"

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QDebug>
#include <QFileInfo>
#include <QHttpMultiPart>
#include <QDateTime>
#include <QUrlQuery>
#include <QMimeDatabase>
#include <QPointer>

#define LOG_DATETIME QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss.zzz")

namespace AtProtocolInterface {

HttpAccessManager *AccessAtProtocol::m_manager = nullptr;

AtProtocolAccount::AtProtocolAccount(QObject *parent) : QObject { parent } { }

const AccountData &AtProtocolAccount::account() const
{
    return m_account;
}

void AtProtocolAccount::setAccount(const AccountData &account)
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

void AtProtocolAccount::setSession(const QString &did, const QString &handle, const QString &email,
                                   const QString &accessJwt, const QString &refresh_jwt)
{
    m_account.did = did;
    m_account.handle = handle;
    m_account.email = email;
    m_account.accessJwt = accessJwt;
    m_account.refreshJwt = refresh_jwt;
}

QString AtProtocolAccount::service() const
{
    return m_account.service;
}

void AtProtocolAccount::setService(const QString &newService)
{
    m_account.service = newService;
}

QString AtProtocolAccount::did() const
{
    return m_account.did;
}

QString AtProtocolAccount::handle() const
{
    return m_account.handle;
}

QString AtProtocolAccount::email() const
{
    return m_account.email;
}

QString AtProtocolAccount::accessJwt() const
{
    return m_account.accessJwt;
}

QString AtProtocolAccount::refreshJwt() const
{
    return m_account.refreshJwt;
}

AccessAtProtocol::AccessAtProtocol(QObject *parent) : AtProtocolAccount { parent }
{
    qDebug().noquote() << LOG_DATETIME << "AccessAtProtocol::AccessAtProtocol()" << this;
    if (m_manager == nullptr) {
        qDebug().noquote() << LOG_DATETIME << this << "new HttpAccessManager()"
                           << QCoreApplication::instance();
        m_manager = new HttpAccessManager(QCoreApplication::instance());
    }
}

void AccessAtProtocol::get(const QString &endpoint, const QUrlQuery &query,
                           const bool with_auth_header)
{
    if (accessJwt().isEmpty() && with_auth_header) {
        qCritical().noquote() << LOG_DATETIME << "AccessAtProtocol::get()"
                              << "Emty accessJwt!";
        return;
    }

    qDebug().noquote() << LOG_DATETIME << "AccessAtProtocol::get()" << this;
    qDebug().noquote() << LOG_DATETIME << "   " << handle();
    qDebug().noquote() << LOG_DATETIME << "   " << endpoint;
    qDebug().noquote() << LOG_DATETIME << "   " << query.toString();

    QUrl url = QString("%1/%2").arg(service(), endpoint);
    url.setQuery(query);
    QNetworkRequest request(url);
    request.setRawHeader(QByteArray("Cache-Control"), QByteArray("no-cache"));
    if (with_auth_header) {
        request.setRawHeader(QByteArray("Authorization"),
                             QByteArray("Bearer ") + accessJwt().toUtf8());
    }

    QPointer<AccessAtProtocol> alive = this;
    HttpReply *reply = m_manager->get(request);
    connect(reply, &HttpReply::finished, [=]() {
        qDebug().noquote() << LOG_DATETIME << reply->error() << reply->url().toString();
        if (alive) {
            qDebug().noquote() << LOG_DATETIME << "  " << this->thread();

            bool success = false;
            if (checkReply(reply)) {
                success = parseJson(true, m_replyJson);
            }
            emit finished(success);
        } else {
            qDebug().noquote() << LOG_DATETIME << "Parent is deleted!!!!!!!!!!";
        }
        reply->deleteLater();
    });
}

void AccessAtProtocol::post(const QString &endpoint, const QByteArray &json,
                            const bool with_auth_header)
{
    qDebug().noquote() << LOG_DATETIME << "AccessAtProtocol::post()" << this;
    qDebug().noquote() << LOG_DATETIME << "   " << handle();
    qDebug().noquote() << LOG_DATETIME << "   " << endpoint;
    qDebug().noquote() << LOG_DATETIME << "   " << json;

    QNetworkRequest request(QUrl(QString("%1/%2").arg(service(), endpoint)));
    request.setRawHeader(QByteArray("Cache-Control"), QByteArray("no-cache"));
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

    QPointer<AccessAtProtocol> alive = this;
    HttpReply *reply = m_manager->post(request, json);
    connect(reply, &HttpReply::finished, [=]() {
        qDebug().noquote() << LOG_DATETIME << reply->error() << reply->url().toString();
        if (alive) {
            bool success = false;
            if (checkReply(reply)) {
                success = parseJson(true, m_replyJson);
            }
            emit finished(success);
        } else {
            qDebug().noquote() << LOG_DATETIME << "Parent is deleted!!!!!!!!!!";
        }
        reply->deleteLater();
    });
}

void AccessAtProtocol::postWithImage(const QString &endpoint, const QString &path)
{
    if (accessJwt().isEmpty()) {
        qCritical().noquote() << LOG_DATETIME << "AccessAtProtocol::postWithImage()"
                              << "Empty accessJwt!";
        return;
    }
    if (!QFile::exists(path)) {
        qCritical().noquote() << LOG_DATETIME << "AccessAtProtocol::postWithImage()"
                              << "Not found" << path;
        return;
    }
    qDebug().noquote() << LOG_DATETIME << "AccessAtProtocol::postWithImage()" << this << endpoint
                       << path;

    QMimeDatabase mime;
    QFileInfo info(path);
    QNetworkRequest request(QUrl(QString("%1/%2").arg(service(), endpoint)));
    request.setRawHeader(QByteArray("Cache-Control"), QByteArray("no-cache"));
    request.setRawHeader(QByteArray("Authorization"), QByteArray("Bearer ") + accessJwt().toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, mime.mimeTypeForFile(info).name());

    QFile *file = new QFile(path);
    if (!file->open(QIODevice::ReadOnly)) {
        qCritical().noquote() << LOG_DATETIME << LOG_DATETIME << "AccessAtProtocol::postWithImage()"
                              << "Not open" << path;
        delete file;
        return;
    }
    request.setHeader(QNetworkRequest::ContentLengthHeader, file->size());

    QPointer<AccessAtProtocol> alive = this;
    HttpReply *reply = m_manager->post(request, file->readAll());
    file->setParent(reply);
    connect(reply, &HttpReply::finished, [=]() {
        qDebug().noquote() << LOG_DATETIME << reply->error() << reply->url().toString();
        if (alive) {
            bool success = false;
            if (checkReply(reply)) {
                success = parseJson(true, m_replyJson);
            }
            emit finished(success);
        } else {
            qDebug().noquote() << LOG_DATETIME << "Parent is deleted!!!!!!!!!!";
        }
        reply->deleteLater();
    });
}

bool AccessAtProtocol::checkReply(HttpReply *reply)
{
    bool status = false;
    m_replyJson = QString::fromUtf8(reply->readAll());
    m_errorCode.clear();
    m_errorMessage.clear();

#ifdef QT_DEBUG
    for (const auto &header : reply->rawHeaderPairs()) {
        if (header.first.toLower().startsWith("ratelimit-")) {
            if (header.first.toLower() == "ratelimit-reset") {
                qDebug().noquote() << LOG_DATETIME << header.first
                                   << QDateTime::fromSecsSinceEpoch(header.second.toInt())
                                              .toString("yyyy/MM/dd hh:mm:ss");
            } else {
                qDebug().noquote() << LOG_DATETIME << header.first << header.second;
            }
        }
    }
#endif

    QJsonDocument json_doc = QJsonDocument::fromJson(m_replyJson.toUtf8());
    if (reply->error() != HttpReply::Success) {
        if (json_doc.object().contains("error") && json_doc.object().contains("error")) {
            m_errorCode = json_doc.object().value("error").toString();
            m_errorMessage = json_doc.object().value("message").toString();
        } else {
            m_errorCode = QStringLiteral("Other");
            m_errorMessage = m_replyJson;
        }
        if (m_errorCode == "RateLimitExceeded") {
            m_errorMessage += "\n";
            for (const auto &header : reply->rawHeaderPairs()) {
                if (header.first.toLower().startsWith("ratelimit-")) {
                    if (header.first.toLower() == "ratelimit-reset") {
                        m_errorMessage += QString("\n%1:%2").arg(
                                header.first,
                                QDateTime::fromSecsSinceEpoch(header.second.toInt())
                                        .toString("yyyy/MM/dd hh:mm:ss"));
                    } else {
                        m_errorMessage += QString("\n%1:%2").arg(header.first, header.second);
                    }
                }
            }
        }
        qCritical().noquote() << LOG_DATETIME << m_errorCode << m_errorMessage;
        qCritical().noquote() << LOG_DATETIME << m_replyJson;
    } else {
        status = true;
    }
    return status;
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
