#include "authorization.h"
#include "http/httpaccess.h"
#include "http/simplehttpserver.h"
#include "atprotocol/com/atproto/repo/comatprotorepodescriberepo.h"
#include "extension/well-known/wellknownoauthprotectedresource.h"
#include "extension/well-known/wellknownoauthauthorizationserver.h"
#include "extension/oauth/oauthpushedauthorizationrequest.h"
#include "extension/oauth/oauthrequesttoken.h"
#include "atprotocol/lexicons_func_unknown.h"
#include "tools/jsonwebtoken.h"

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
using AtProtocolInterface::OauthPushedAuthorizationRequest;
using AtProtocolInterface::OauthRequestToken;
using AtProtocolInterface::WellKnownOauthAuthorizationServer;
using AtProtocolInterface::WellKnownOauthProtectedResource;

Authorization::Authorization(QObject *parent) : QObject { parent }, m_redirectTimeout(300) { }

void Authorization::reset()
{
    // user
    m_handle.clear();
    // server info
    m_serviceEndpoint.clear();
    m_authorizationServer.clear();
    // server meta data
    m_pushedAuthorizationRequestEndpoint.clear();
    m_authorizationEndpoint.clear();
    m_tokenEndopoint.clear();
    m_scopesSupported.clear();
    //
    m_redirectUri.clear();
    m_clientId.clear();
    // par
    m_codeChallenge.clear();
    m_codeVerifier.clear();
    m_state.clear();
    m_parPayload.clear();
    // request token
    m_code.clear();
    m_requestTokenPayload.clear();
    m_token = AtProtocolType::OauthDefs::TokenResponse();
    //
    m_listenPort.clear();
}

void Authorization::start(const QString &pds, const QString &handle)
{
    if (pds.isEmpty() || handle.isEmpty())
        return;

    startRedirectServer();

    AtProtocolInterface::AccountData account;
    account.service = pds;
    m_handle = handle;

    ComAtprotoRepoDescribeRepo *repo = new ComAtprotoRepoDescribeRepo(this);
    connect(repo, &ComAtprotoRepoDescribeRepo::finished, this, [=](bool success) {
        if (success) {
            auto did_doc = AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                    AtProtocolType::DirectoryPlcDefs::DidDoc>(repo->didDoc());
            if (!did_doc.service.isEmpty()) {
                setServiceEndpoint(did_doc.service.first().serviceEndpoint);

                // next step
                requestOauthProtectedResource();
            } else {
                emit errorOccured("Invalid oauth-protected-resource",
                                  "authorization_servers is empty.");
                emit finished(false);
            }
        } else {
            emit errorOccured(repo->errorCode(), repo->errorMessage());
            emit finished(false);
        }
        repo->deleteLater();
    });
    repo->setAccount(account);
    repo->describeRepo(handle);
}

void Authorization::requestOauthProtectedResource()
{
    // /.well-known/oauth-protected-resource
    if (serviceEndpoint().isEmpty())
        return;

    AtProtocolInterface::AccountData account;
    account.service = serviceEndpoint();

    WellKnownOauthProtectedResource *resource = new WellKnownOauthProtectedResource(this);
    connect(resource, &WellKnownOauthProtectedResource::finished, this, [=](bool success) {
        if (success) {
            if (!resource->resourceMetadata().authorization_servers.isEmpty()) {
                setAuthorizationServer(resource->resourceMetadata().authorization_servers.first());
                // next step
                requestOauthAuthorizationServer();
            } else {
                emit errorOccured("Invalid oauth-protected-resource",
                                  "authorization_servers is empty.");
                emit finished(false);
            }
        } else {
            emit errorOccured(resource->errorCode(), resource->errorMessage());
            emit finished(false);
        }
        resource->deleteLater();
    });
    resource->setAccount(account);
    resource->oauthProtectedResource();
}

void Authorization::requestOauthAuthorizationServer()
{
    // /.well-known/oauth-authorization-server

    if (authorizationServer().isEmpty())
        return;

    AtProtocolInterface::AccountData account;
    account.service = authorizationServer();

    WellKnownOauthAuthorizationServer *server = new WellKnownOauthAuthorizationServer(this);
    connect(server, &WellKnownOauthAuthorizationServer::finished, this, [=](bool success) {
        if (success) {
            QString error_message;
            if (validateServerMetadata(server->serverMetadata(), error_message)) {
                setPushedAuthorizationRequestEndpoint(
                        server->serverMetadata().pushed_authorization_request_endpoint);
                setAuthorizationEndpoint(server->serverMetadata().authorization_endpoint);
                setTokenEndopoint(server->serverMetadata().token_endpoint);
                m_scopesSupported = server->serverMetadata().scopes_supported;

                // next step
                makeParPayload();
                par();
            } else {
                qDebug().noquote() << error_message;
                emit errorOccured("Invalid oauth-authorization-server", error_message);
                emit finished(false);
            }
        } else {
            emit errorOccured(server->errorCode(), server->errorMessage());
            emit finished(false);
        }
        server->deleteLater();
    });
    server->setAccount(account);
    server->oauthAuthorizationServer();
}

bool Authorization::validateServerMetadata(
        const AtProtocolType::WellKnownDefs::ServerMetadata &server_metadata,
        QString &error_message)
{
    bool ret = false;
    if (QUrl(server_metadata.issuer).host() != QUrl(authorizationServer()).host()) {
        // リダイレクトされると変わるので対応しないといけない
        error_message = QString("'issuer' is an invalid value(%1).").arg(server_metadata.issuer);
    } else if (!server_metadata.response_types_supported.contains("code")) {
        error_message = QStringLiteral("'response_types_supported' must contain 'code'.");
    } else if (!server_metadata.grant_types_supported.contains("authorization_code")) {
        error_message =
                QStringLiteral("'grant_types_supported' must contain 'authorization_code'.");
    } else if (!server_metadata.grant_types_supported.contains("refresh_token")) {
        error_message = QStringLiteral("'grant_types_supported' must contain 'refresh_token'.");
    } else if (!server_metadata.code_challenge_methods_supported.contains("S256")) {
        error_message = QStringLiteral("'code_challenge_methods_supported' must contain 'S256'.");
    } else if (!server_metadata.token_endpoint_auth_methods_supported.contains("private_key_jwt")) {
        error_message = QStringLiteral(
                "'token_endpoint_auth_methods_supported' must contain 'private_key_jwt'.");
    } else if (!server_metadata.token_endpoint_auth_methods_supported.contains("none")) {
        error_message =
                QStringLiteral("'token_endpoint_auth_methods_supported' must contain 'none'.");
    } else if (!server_metadata.token_endpoint_auth_signing_alg_values_supported.contains(
                       "ES256")) {
        error_message = QStringLiteral(
                "'token_endpoint_auth_signing_alg_values_supported' must contain 'ES256'.");
    } else if (!server_metadata.scopes_supported.contains("atproto")) {
        error_message = QStringLiteral("'scopes_supported' must contain 'atproto'.");
    } else if (!(server_metadata.subject_types_supported.isEmpty()
                 || (!server_metadata.subject_types_supported.isEmpty()
                     && server_metadata.subject_types_supported.contains("public")))) {
        error_message = QStringLiteral("'subject_types_supported' must contain 'public'.");
    } else if (!server_metadata.authorization_response_iss_parameter_supported) {
        error_message =
                QString("'authorization_response_iss_parameter_supported' is an invalid value(%1).")
                        .arg(server_metadata.authorization_response_iss_parameter_supported);
    } else if (server_metadata.pushed_authorization_request_endpoint.isEmpty()) {
        error_message = QStringLiteral("pushed_authorization_request_endpoint must be set'.");
    } else if (!server_metadata.require_pushed_authorization_requests) {
        error_message = QString("'require_pushed_authorization_requests' is an invalid value(%1).")
                                .arg(server_metadata.require_pushed_authorization_requests);
    } else if (!server_metadata.dpop_signing_alg_values_supported.contains("ES256")) {
        error_message = QStringLiteral("'dpop_signing_alg_values_supported' must contain 'ES256'.");
    } else if (!server_metadata.require_request_uri_registration) {
        error_message = QString("'require_request_uri_registration' is an invalid value(%1).")
                                .arg(server_metadata.require_request_uri_registration);
    } else if (!server_metadata.client_id_metadata_document_supported) {
        error_message = QString("'client_id_metadata_document_supported' is an invalid value(%1).")
                                .arg(server_metadata.client_id_metadata_document_supported);
    } else {
        ret = true;
    }
    return ret;
}

QByteArray Authorization::state() const
{
    return m_state;
}

QString Authorization::listenPort() const
{
    return m_listenPort;
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
    m_codeVerifier = generateRandomValues().toBase64(QByteArray::Base64UrlEncoding
                                                     | QByteArray::OmitTrailingEquals);
    m_codeChallenge =
            QCryptographicHash::hash(m_codeVerifier, QCryptographicHash::Sha256)
                    .toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
}

void Authorization::makeParPayload()
{
    makeClientId();
    makeCodeChallenge();
    m_state = QCryptographicHash::hash(m_codeVerifier, QCryptographicHash::Sha256)
                      .toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);

    QString login_hint = m_handle;

    QUrlQuery query;
    query.addQueryItem("response_type", "code");
    query.addQueryItem("code_challenge", m_codeChallenge);
    query.addQueryItem("code_challenge_method", "S256");
    query.addQueryItem("client_id", simplyEncode(m_clientId));
    query.addQueryItem("state", m_state);
    query.addQueryItem("redirect_uri", simplyEncode(m_redirectUri));
    query.addQueryItem("scope", m_scopesSupported.join(" "));
    query.addQueryItem("login_hint", simplyEncode(login_hint));

    m_parPayload = query.query(QUrl::FullyEncoded).toLocal8Bit();
}

void Authorization::par()
{
    if (m_parPayload.isEmpty() || pushedAuthorizationRequestEndpoint().isEmpty())
        return;

    AtProtocolInterface::AccountData account;
    account.service = pushedAuthorizationRequestEndpoint();

    OauthPushedAuthorizationRequest *req = new OauthPushedAuthorizationRequest(this);
    connect(req, &OauthPushedAuthorizationRequest::finished, this, [=](bool success) {
        if (success) {
            if (!req->pushedAuthorizationResponse().request_uri.isEmpty()) {
                // next step
                setDPopNonce(req->dPopNonce());
                authorization(req->pushedAuthorizationResponse().request_uri);
            } else {
                emit errorOccured("Invalid Pushed Authorization Request",
                                  "'request_uri' is empty.");
                emit finished(false);
            }
        } else {
            emit errorOccured(req->errorCode(), req->errorMessage());
            emit finished(false);
        }
        req->deleteLater();
    });
    req->setContentType("application/x-www-form-urlencoded");
    req->setAccount(account);
    req->pushedAuthorizationRequest(m_parPayload);
}

void Authorization::authorization(const QString &request_uri)
{
    if (request_uri.isEmpty() || authorizationEndpoint().isEmpty() || m_listenPort.isEmpty())
        return;

    QString authorization_endpoint = authorizationEndpoint();

    QUrl url(authorization_endpoint);
    QUrlQuery query;
    query.addQueryItem("client_id", simplyEncode(m_clientId));
    query.addQueryItem("request_uri", simplyEncode(request_uri));
    url.setQuery(query);

    qDebug().noquote() << "redirect" << url.toEncoded();

#ifdef HAGOROMO_UNIT_TEST
    emit madeRequestUrl(url.toString());
#else
    QDesktopServices::openUrl(url);
#endif
}

void Authorization::startRedirectServer()
{
    SimpleHttpServer *server = new SimpleHttpServer(this);
    connect(server, &SimpleHttpServer::received, this,
            [=](const QHttpServerRequest &request, bool &result, QByteArray &data,
                QByteArray &mime_type) {
                qDebug().noquote() << "receive by startRedirectServer";
                qDebug().noquote() << "  " << request.url().toString();
                qDebug().noquote() << "  " << request.url().path();

                if (request.url().path() == "/tech/relog/hagoromo/oauth-callback") { }

                if (request.query().hasQueryItem("iss") && request.query().hasQueryItem("state")
                    && request.query().hasQueryItem("code")) {
                    // authorize
                    QString state = request.query().queryItemValue("state");
                    result = (state.toUtf8() == m_state);
                    if (result) {
                        m_code = request.query().queryItemValue("code").toUtf8();
                        requestToken();
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
                QTimer::singleShot(10 * 1000, [=]() { server->deleteLater(); });
            });
    connect(server, &SimpleHttpServer::timeout, this, [=]() {
        // token取得に進んでたらfinishedは発火しない
        if (m_code.isEmpty()) {
            qDebug().noquote() << "Authorization timeout";
            emit finished(false);
        }
        server->deleteLater();
    });
    server->setTimeout(redirectTimeout());
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

void Authorization::requestToken()
{
    if (tokenEndopoint().isEmpty())
        return;

    makeRequestTokenPayload();

    AtProtocolInterface::AccountData account;
    account.service = tokenEndopoint();

    OauthRequestToken *req = new OauthRequestToken(this);
    connect(req, &OauthRequestToken::finished, this, [=](bool success) {
        bool ret = false;
        if (success) {
            if (!req->tokenResponse().access_token.isEmpty()
                && !req->tokenResponse().refresh_token.isEmpty()
                && req->tokenResponse().token_type.toLower() == "dpop") {

                if (!req->dPopNonce().isEmpty()) {
                    setDPopNonce(req->dPopNonce());
                }
                setToken(req->tokenResponse());

                qDebug().noquote() << "--- Success oauth ----";
                qDebug().noquote() << "  handle :" << m_handle;
                qDebug().noquote() << "  access :" << m_token.access_token;
                qDebug().noquote() << "  refresh:" << m_token.refresh_token;
                qDebug().noquote() << req->replyJson();
                qDebug().noquote() << "----------------------";
                // finish oauth sequence
                ret = true;
            } else {
                emit errorOccured("Invalid token response", req->replyJson());
            }
        } else {
            emit errorOccured(req->errorCode(), req->errorMessage());
        }
        emit finished(ret);
        req->deleteLater();
    });
    req->appendRawHeader("DPoP",
                         JsonWebToken::generate(tokenEndopoint(), m_clientId, "POST", dPopNonce()));
    req->setContentType("application/x-www-form-urlencoded");
    req->setAccount(account);
    req->requestToken(m_requestTokenPayload);
}

QByteArray Authorization::generateRandomValues() const
{
    QByteArray values;
#ifdef HAGOROMO_UNIT_TEST_
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

QString Authorization::serviceEndpoint() const
{
    return m_serviceEndpoint;
}

void Authorization::setServiceEndpoint(const QString &newServiceEndpoint)
{
    if (m_serviceEndpoint == newServiceEndpoint)
        return;
    m_serviceEndpoint = newServiceEndpoint;
    emit serviceEndpointChanged();
}

QString Authorization::authorizationServer() const
{
    return m_authorizationServer;
}

void Authorization::setAuthorizationServer(const QString &newAuthorizationServer)
{
    if (m_authorizationServer == newAuthorizationServer)
        return;
    m_authorizationServer = newAuthorizationServer;
    emit authorizationServerChanged();
}

QString Authorization::pushedAuthorizationRequestEndpoint() const
{
    return m_pushedAuthorizationRequestEndpoint;
}

void Authorization::setPushedAuthorizationRequestEndpoint(
        const QString &newPushedAuthorizationRequestEndpoint)
{
    if (m_pushedAuthorizationRequestEndpoint == newPushedAuthorizationRequestEndpoint)
        return;
    m_pushedAuthorizationRequestEndpoint = newPushedAuthorizationRequestEndpoint;
    emit pushedAuthorizationRequestEndpointChanged();
}

QString Authorization::authorizationEndpoint() const
{
    return m_authorizationEndpoint;
}

void Authorization::setAuthorizationEndpoint(const QString &newAuthorizationEndpoint)
{
    if (m_authorizationEndpoint == newAuthorizationEndpoint)
        return;
    m_authorizationEndpoint = newAuthorizationEndpoint;
    emit authorizationEndpointChanged();
}

QString Authorization::tokenEndopoint() const
{
    return m_tokenEndopoint;
}

void Authorization::setTokenEndopoint(const QString &newTokenEndopoint)
{
    if (m_tokenEndopoint == newTokenEndopoint)
        return;
    m_tokenEndopoint = newTokenEndopoint;
    emit tokenEndopointChanged();
}

int Authorization::redirectTimeout() const
{
    return m_redirectTimeout;
}

void Authorization::setRedirectTimeout(int newRedirectTimeout)
{
    m_redirectTimeout = newRedirectTimeout;
}

AtProtocolType::OauthDefs::TokenResponse Authorization::token() const
{
    return m_token;
}

void Authorization::setToken(const AtProtocolType::OauthDefs::TokenResponse &newToken)
{
    if (m_token.access_token == newToken.access_token && m_token.expires_in == newToken.expires_in
        && m_token.refresh_token == newToken.refresh_token
        && m_token.token_type == newToken.token_type)
        return;
    m_token = newToken;
    emit tokenChanged();
}

QString Authorization::dPopNonce() const
{
    return m_dPopNonce;
}

void Authorization::setDPopNonce(const QString &newDPopNonce)
{
    m_dPopNonce = newDPopNonce;
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
