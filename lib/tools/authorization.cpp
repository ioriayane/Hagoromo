#include "authorization.h"
#include "http/httpaccess.h"
#include "http/simplehttpserver.h"
#include "atprotocol/com/atproto/repo/comatprotorepodescriberepo.h"
#include "atprotocol/lexicons_func_unknown.h"

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

using AtProtocolInterface::ComAtprotoRepoDescribeRepo;

Authorization::Authorization(QObject *parent) : QObject { parent } { }

void Authorization::reset()
{
    m_serviceEndpoint.clear();
    m_redirectUri.clear();
    m_clientId.clear();
    m_codeChallenge.clear();
    m_codeVerifier.clear();
    m_state.clear();
    m_parPayload.clear();
    m_requestTokenPayload.clear();
}

void Authorization::start(const QString &pds, const QString &handle)
{
    if (pds.isEmpty() || handle.isEmpty())
        return;

    AtProtocolInterface::AccountData account;
    account.service = pds;

    ComAtprotoRepoDescribeRepo *repo = new ComAtprotoRepoDescribeRepo(this);
    connect(repo, &ComAtprotoRepoDescribeRepo::finished, this, [=](bool success) {
        if (success) {
            auto did_doc = AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                    AtProtocolType::DirectoryPlcDefs::DidDoc>(repo->didDoc());
            if (!did_doc.service.isEmpty()) {
                setServiceEndpoint(did_doc.service.first().serviceEndpoint);

                // next step
            }
        } else {
            emit errorOccured(repo->errorCode(), repo->errorMessage());
        }
        repo->deleteLater();
    });
    repo->setAccount(account);
    repo->describeRepo(handle);
}

void Authorization::makeClientId()
{
    QString port;
    if (!m_listenPort.isEmpty()) {
        port.append(":");
        port.append(m_listenPort);
    }
    m_redirectUri.append("http://127.0.0.1");
    m_redirectUri.append(port);
    m_redirectUri.append("/tech/relog/hagoromo/oauth-callback");
    m_clientId.append("http://localhost/tech/relog/hagoromo?redirect_uri=");
    m_clientId.append(simplyEncode(m_redirectUri));
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
    makeClientId();
    makeCodeChallenge();
    m_state = QCryptographicHash::hash(m_codeVerifier, QCryptographicHash::Sha256)
                      .toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);

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
    query.addQueryItem("client_id", simplyEncode(m_clientId));
    query.addQueryItem("state", m_state);
    query.addQueryItem("redirect_uri", simplyEncode(m_redirectUri));
    query.addQueryItem("scope", scopes_supported.join(" "));
    query.addQueryItem("login_hint", simplyEncode(login_hint));

    m_parPayload = query.query(QUrl::FullyEncoded).toLocal8Bit();
}

void Authorization::par()
{
    if (m_parPayload.isEmpty())
        return;

    QString endpoint = "https://bsky.social/oauth/par";
    QNetworkRequest request((QUrl(endpoint)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QPointer<Authorization> alive = this;
    HttpAccess *access = new HttpAccess(this);
    HttpReply *reply = new HttpReply(access);
    reply->setOperation(HttpReply::Operation::PostOperation);
    reply->setRequest(request);
    reply->setSendData(m_parPayload);
    connect(access, &HttpAccess::finished, this, [access, alive, this](HttpReply *reply) {
        if (alive && reply != nullptr) {
            qDebug().noquote() << reply->error() << reply->url().toString();
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
                emit finished(false);
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
                if (request.query().hasQueryItem("iss") && request.query().hasQueryItem("state")
                    && request.query().hasQueryItem("code")) {
                    // authorize
                    QString state = request.query().queryItemValue("state");
                    result = (state.toUtf8() == m_state);
                    if (result) {
                        m_code = request.query().queryItemValue("code").toUtf8();
                        // requestToken();
                    } else {
                        qDebug().noquote() << "Unknown state in authorization redirect :" << state;
                        emit finished(false);
                        m_code.clear();
                    }
                }
                if (result) {
                    SimpleHttpServer::readFile(":/tools/oauth/oauth_success.html", data);
                } else {
                    SimpleHttpServer::readFile(":/tools/oauth/oauth_fail.html", data);
                }
                mime_type = "text/html";

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

void Authorization::makeRequestTokenPayload()
{
    QUrlQuery query;

    query.addQueryItem("grant_type", "authorization_code");
    query.addQueryItem("code", m_code);
    query.addQueryItem("code_verifier", m_codeVerifier);
    query.addQueryItem("client_id", simplyEncode(m_clientId));
    query.addQueryItem("redirect_uri", simplyEncode(m_redirectUri));

    m_requestTokenPayload = query.query(QUrl::FullyEncoded).toLocal8Bit();
}

bool Authorization::requestToken()
{

    QString endpoint = "https://bsky.social/oauth/token";
    QNetworkRequest request((QUrl(endpoint)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader(QByteArray("DPoP"), QByteArray(""));

    QPointer<Authorization> alive = this;
    HttpAccess *access = new HttpAccess(this);
    HttpReply *reply = new HttpReply(access);
    reply->setOperation(HttpReply::Operation::PostOperation);
    reply->setRequest(request);
    // reply->setSendData(m_parPayload);
    connect(access, &HttpAccess::finished, this, [access, alive, this](HttpReply *reply) {
        if (alive && reply != nullptr) {
            qDebug().noquote() << reply->error() << reply->url().toString();

            qDebug().noquote() << reply->contentType();
            qDebug().noquote() << reply->readAll();
            if (reply->error() == HttpReply::Success) {
                QJsonDocument json_doc = QJsonDocument::fromJson(reply->readAll());

                qDebug().noquote()
                        << "request_uri" << json_doc.object().value("request_uri").toString();
            } else {
                // error
                qDebug() << "Request token Error";
            }
        } else {
            qDebug().noquote() << "Parent is deleted or reply null !!!!!!!!!!";
        }
        access->deleteLater();
    });
    qDebug() << "request token 1";
    access->process(reply);
    qDebug() << "request token 2";

    return true;
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

void Authorization::setServiceEndpoint(const QString &newServiceEndpoint)
{
    if (m_serviceEndpoint == newServiceEndpoint)
        return;
    m_serviceEndpoint = newServiceEndpoint;
    emit serviceEndpointChanged();
}

QString Authorization::serviceEndpoint() const
{
    return m_serviceEndpoint;
}

QByteArray Authorization::ParPayload() const
{
    return m_parPayload;
}

QByteArray Authorization::codeChallenge() const
{
    return m_codeChallenge;
}

QByteArray Authorization::codeVerifier() const
{
    return m_codeVerifier;
}
