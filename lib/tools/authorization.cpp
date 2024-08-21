#include "authorization.h"
#include "http/httpaccess.h"
#include "http/simplehttpserver.h"

#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QUrl>
#include <QUrlQuery>
#include <QDebug>
#include <QNetworkRequest>
#include <QPointer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDesktopServices>
#include <QTimer>

Authorization::Authorization(QObject *parent) : QObject { parent } { }

void Authorization::reset()
{
    m_codeChallenge.clear();
    m_codeVerifier.clear();
}

void Authorization::makeCodeChallenge()
{
    m_codeChallenge = generateRandomValues().toBase64(QByteArray::Base64UrlEncoding
                                                      | QByteArray::OmitTrailingEquals);
    m_codeVerifier =
            QCryptographicHash::hash(m_codeChallenge, QCryptographicHash::Sha256)
                    .toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
}

void Authorization::makeParPayload()
{
    makeCodeChallenge();
    m_state = QCryptographicHash::hash(m_codeVerifier, QCryptographicHash::Sha256)
                      .toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);

    QString redirect_uri = "http://127.0.0.1:8080/tech/relog/hagoromo/oauth-callback";
    QString client_id = "http://localhost/tech/relog/"
                        "hagoromo?redirect_uri="
            + simplyEncode(redirect_uri);
    QStringList scopes_supported;
    scopes_supported << "offline_access"
                     << "openid"
                     // << "email"
                     // << "phone"
                     << "profile";
    QString login_hint = "ioriayane2.bsky.social";

    QUrlQuery query;
    query.addQueryItem("response_type", "code");
    query.addQueryItem("code_challenge", m_codeChallenge);
    query.addQueryItem("code_challenge_method", "S256");
    query.addQueryItem("client_id", simplyEncode(client_id));
    query.addQueryItem("state", m_state);
    query.addQueryItem("redirect_uri", simplyEncode(redirect_uri));
    query.addQueryItem("scope", scopes_supported.join(" "));
    query.addQueryItem("login_hint", simplyEncode(login_hint));

    m_parPlayload = query.query(QUrl::FullyEncoded).toLocal8Bit();
}

void Authorization::par()
{
    if (m_parPlayload.isEmpty())
        return;

    QString endpoint = "https://bsky.social/oauth/par";
    QNetworkRequest request((QUrl(endpoint)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QPointer<Authorization> alive = this;
    HttpAccess *access = new HttpAccess(this);
    HttpReply *reply = new HttpReply(access);
    reply->setOperation(HttpReply::Operation::PostOperation);
    reply->setRequest(request);
    reply->setSendData(m_parPlayload);
    connect(access, &HttpAccess::finished, this, [access, alive, this](HttpReply *reply) {
        if (alive && reply != nullptr) {
            qDebug().noquote() << reply->error() << reply->url().toString();
            // emit finished(success);
            qDebug().noquote() << reply->contentType();
            qDebug().noquote() << reply->readAll();
            if (reply->error() == HttpReply::Success) {
                QJsonDocument json_doc = QJsonDocument::fromJson(reply->readAll());

                qDebug().noquote()
                        << "request_uri" << json_doc.object().value("request_uri").toString();
                authorization(json_doc.object().value("request_uri").toString());
            } else {
                // error
                qDebug() << "PAR Error";
            }
        } else {
            qDebug().noquote() << "Parent is deleted or reply null !!!!!!!!!!";
        }
        access->deleteLater();
    });
    access->process(reply);
}

void Authorization::authorization(const QString &request_uri)
{
    if (request_uri.isEmpty())
        return;

    QString authorization_endpoint = "https://bsky.social/oauth/authorize";
    QString redirect_uri = "http://127.0.0.1:8080/tech/relog/hagoromo/oauth-callback";
    QString client_id = "http://localhost/tech/relog/"
                        "hagoromo?redirect_uri="
            + simplyEncode(redirect_uri);

    QUrl url(authorization_endpoint);
    QUrlQuery query;
    query.addQueryItem("client_id", simplyEncode(client_id));
    query.addQueryItem("request_uri", simplyEncode(request_uri));
    url.setQuery(query);

    qDebug().noquote() << "redirect" << url.toEncoded();

    QDesktopServices::openUrl(url);
}

void Authorization::startRedirectServer()
{
    SimpleHttpServer *server = new SimpleHttpServer(this);
    connect(server, &SimpleHttpServer::received, this,
            [=](const QHttpServerRequest &request, bool &result, QByteArray &data,
                QByteArray &mime_type) {
                SimpleHttpServer::readFile(":/tools/oauth/oauth_success.html", data);
                mime_type = "text/html";
                result = true;

                // TODO : verify url and parameters
                requestToken();

                // delete after 10 sec.
                QTimer::singleShot(10 * 1000, [=]() {
                    emit finished(true); // temporary
                    server->deleteLater();
                });
            });
    connect(server, &SimpleHttpServer::timeout, this, [=]() {
        // token取得に進んでたらfinishedは発火しない
        qDebug().noquote() << "Authorization timeout";
        emit finished(false);
        server->deleteLater();
    });
    server->setTimeout(50); // 300);
    quint16 port = server->listen(QHostAddress::LocalHost, 0);
    m_listenPort = QString::number(port);

    qDebug().noquote() << "Listen" << m_listenPort;
}

void Authorization::requestToken()
{

    //
}

QByteArray Authorization::generateRandomValues() const
{
    QByteArray values;
#ifdef HAGOROMO_UNIT_TEST1
    const uint8_t base[] = { 116, 24,  223, 180, 151, 153, 224, 37,  79,  250, 96,
                             125, 216, 173, 187, 186, 22,  212, 37,  77,  105, 214,
                             191, 240, 91,  88,  5,   88,  83,  132, 141, 121 };
    for (int i = 0; i < sizeof(base); i++) {
        values.append(base[i]);
    }
#else
    for (int i = 0; i < 32; i++) {
        values.append(static_cast<char>(QRandomGenerator::global()->bounded(256)));
    }
#endif
    return values;
}

QString Authorization::simplyEncode(QString text) const
{
    return text.replace("%", "%25").replace(":", "%3A").replace("/", "%2F").replace("?", "%3F");
}

QByteArray Authorization::ParPlayload() const
{
    return m_parPlayload;
}

QByteArray Authorization::codeChallenge() const
{
    return m_codeChallenge;
}

QByteArray Authorization::codeVerifier() const
{
    return m_codeVerifier;
}
