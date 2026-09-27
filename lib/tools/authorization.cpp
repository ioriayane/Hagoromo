#include "authorization.h"
#include "http/httpaccess.h"
#include "http/simplehttpserver.h"
#include "extension/well-known/wellknownoauthprotectedresource.h"
#include "extension/well-known/wellknownoauthauthorizationserver.h"
#include "extension/oauth/oauthpushedauthorizationrequest.h"
#include "extension/oauth/oauthrequesttoken.h"
#include "tools/jsonwebtoken.h"
#include "tools/identityresolver.h"

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

using AtProtocolInterface::OauthPushedAuthorizationRequest;
using AtProtocolInterface::OauthRequestToken;
using AtProtocolInterface::WellKnownOauthAuthorizationServer;
using AtProtocolInterface::WellKnownOauthProtectedResource;

// issuerはメタデータを取得したAuthorization Serverのorigin(scheme/host/port)と一致すること
// issuer自体はoriginなのでパスなどを含まない
inline bool isSameOrigin(const QString &issuer, const QString &authorization_server)
{
    const QUrl issuer_url(issuer);
    const QUrl server_url(authorization_server);
    if (!issuer_url.isValid() || issuer_url.host().isEmpty() || !server_url.isValid())
        return false;
    if (!(issuer_url.path().isEmpty() || issuer_url.path() == "/") || issuer_url.hasQuery()
        || issuer_url.hasFragment() || !issuer_url.userInfo().isEmpty())
        return false;
    const int issuer_default_port = (issuer_url.scheme() == "https") ? 443 : 80;
    const int server_default_port = (server_url.scheme() == "https") ? 443 : 80;
    return issuer_url.scheme() == server_url.scheme() && issuer_url.host() == server_url.host()
            && issuer_url.port(issuer_default_port) == server_url.port(server_default_port);
}

// Authorization ServerからDPoP nonceの更新を求められた場合、新しいnonceで1回だけ再送する
inline bool canRetryWithDPopNonce(const QString &error_code, const QString &new_nonce, bool retried)
{
    return !retried && error_code == QStringLiteral("use_dpop_nonce") && !new_nonce.isEmpty();
}

Authorization::Authorization(QObject *parent)
    : QObject { parent },
      m_scopes(defaultScopes()),
      m_plcDirectory(QStringLiteral("https://plc.directory")),
      m_tokenRequesting(false),
      m_redirectTimeout(300)
{
}

void Authorization::reset()
{
    // user
    m_handle.clear();
    m_did.clear();
    // server info
    m_serviceEndpoint.clear();
    m_authorizationServer.clear();
    // server meta data
    m_issuer.clear();
    m_pushedAuthorizationRequestEndpoint.clear();
    m_authorizationEndpoint.clear();
    m_tokenEndopoint.clear();
    //
    m_redirectUri.clear();
    m_clientId.clear();
    // par
    m_codeChallenge.clear();
    m_codeVerifier.clear();
    m_state.clear();
    // request token
    m_code.clear();
    m_token = AtProtocolType::OauthDefs::TokenResponse();
    m_dPopKey.clear();
    //
    m_listenPort.clear();
}

void Authorization::start(const QString &pds, const QString &handle)
{
    if (pds.isEmpty() || handle.isEmpty())
        return;

    // 新しいセッションなのでDPoPの鍵も新しく作る
    if (!m_dPopKey.generateKey()) {
        emit errorOccurred("Invalid DPoP key", "Failed to generate DPoP key.");
        emit finished(false);
        return;
    }

    startRedirectServer();

    // handle(またはDID) -> DID -> DIDドキュメント -> PDS を双方向で検証しながら解決する
    // pdsはDNS/HTTPSでハンドルを解決できなかったときの問い合わせ先
    IdentityResolver *resolver = new IdentityResolver(this);
    connect(resolver, &IdentityResolver::errorOccurred, this, &Authorization::errorOccurred);
    connect(resolver, &IdentityResolver::finished, this, [=](bool success) {
        if (success) {
            // tokenのsubと照合するため
            m_did = resolver->did();
            m_handle = resolver->handle();
            setServiceEndpoint(resolver->pdsEndpoint());

            // next step
            requestOauthProtectedResource();
        } else {
            emit finished(false);
        }
        resolver->deleteLater();
    });
    resolver->setHandleResolutionService(pds);
    resolver->setPlcDirectory(plcDirectory());
    resolver->resolve(handle);
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
                emit errorOccurred("Invalid oauth-protected-resource",
                                   "authorization_servers is empty.");
                emit finished(false);
            }
        } else {
            emit errorOccurred(resource->errorCode(), resource->errorMessage());
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
                m_issuer = server->serverMetadata().issuer;
                setPushedAuthorizationRequestEndpoint(
                        server->serverMetadata().pushed_authorization_request_endpoint);
                setAuthorizationEndpoint(server->serverMetadata().authorization_endpoint);
                setTokenEndopoint(server->serverMetadata().token_endpoint);
                // next step
                par();
            } else {
                qDebug().noquote() << error_message;
                emit errorOccurred("Invalid oauth-authorization-server", error_message);
                emit finished(false);
            }
        } else {
            emit errorOccurred(server->errorCode(), server->errorMessage());
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
    if (!isSameOrigin(server_metadata.issuer, authorizationServer())) {
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

QString Authorization::issuer() const
{
    return m_issuer;
}

QString Authorization::did() const
{
    return m_did;
}

QString Authorization::handle() const
{
    return m_handle;
}

QStringList Authorization::defaultScopes()
{
    const QString appview = QStringLiteral("aud=did:web:api.bsky.app%23bsky_appview");
    const QString chat = QStringLiteral("aud=did:web:api.bsky.chat%23bsky_chat");
    return QStringList() << QStringLiteral("atproto")
                         << QStringLiteral("include:app.bsky.authFullApp?") + appview
                         << QStringLiteral("include:chat.bsky.authFullChatClient?") + chat
                         // 下書きはapp.bsky.authFullAppに含まれていない
                         << QStringLiteral("rpc:app.bsky.draft.createDraft?") + appview
                         << QStringLiteral("rpc:app.bsky.draft.deleteDraft?") + appview
                         << QStringLiteral("rpc:app.bsky.draft.getDrafts?") + appview
                         << QStringLiteral("rpc:app.bsky.draft.updateDraft?") + appview
                         << QStringLiteral("repo:tech.tokimeki.poll.poll")
                         << QStringLiteral("repo:tech.tokimeki.poll.vote")
                         << QStringLiteral("blob:*/*")
                         // ラベラーへの通報
                         << QStringLiteral("rpc:com.atproto.moderation.createReport?aud=*");
}

QStringList Authorization::scopes() const
{
    return m_scopes;
}

void Authorization::setScopes(const QStringList &newScopes)
{
    m_scopes = newScopes;
}

QString Authorization::plcDirectory() const
{
    return m_plcDirectory;
}

void Authorization::setPlcDirectory(const QString &newPlcDirectory)
{
    m_plcDirectory = newPlcDirectory;
}

void Authorization::setListenPort(const QString &newListenPort)
{
    m_listenPort = newListenPort;
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
    m_clientId = "https://oauth.hagoromo.relog.tech/oauth-client-metadata.json";
}

void Authorization::makeCodeChallenge()
{
    m_codeVerifier = generateRandomValues().toBase64(QByteArray::Base64UrlEncoding
                                                     | QByteArray::OmitTrailingEquals);
    m_codeChallenge =
            QCryptographicHash::hash(m_codeVerifier, QCryptographicHash::Sha256)
                    .toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
}

QByteArray Authorization::makeParPayload()
{
    makeClientId();
    makeCodeChallenge();
    // PKCEとは独立した乱数にする
    m_state = generateRandomValues().toBase64(QByteArray::Base64UrlEncoding
                                              | QByteArray::OmitTrailingEquals);

    // ハンドルを検証できなかった場合はDIDで指定する
    QString login_hint = m_handle.isEmpty() ? m_did : m_handle;

    QUrlQuery query;
    query.addQueryItem("response_type", "code");
    query.addQueryItem("code_challenge", m_codeChallenge);
    query.addQueryItem("code_challenge_method", "S256");
    query.addQueryItem("client_id", simplyEncode(m_clientId));
    query.addQueryItem("state", m_state);
    query.addQueryItem("redirect_uri", simplyEncode(m_redirectUri));
    // scopeは ? = & % などを含むのでそのまま届くようにすべてエンコードする
    query.addQueryItem("scope", QString::fromUtf8(QUrl::toPercentEncoding(m_scopes.join(" "))));
    query.addQueryItem("login_hint", simplyEncode(login_hint));

    return query.query(QUrl::FullyEncoded).toLocal8Bit();
}

void Authorization::par()
{
    if (pushedAuthorizationRequestEndpoint().isEmpty())
        return;
    if (!m_dPopKey.isValid() && !m_dPopKey.generateKey()) {
        emit errorOccurred("Invalid DPoP key", "Failed to generate DPoP key.");
        emit finished(false);
        return;
    }

    // 再送時にcode_challengeやstateが変わらないようにペイロードは1回だけ作る
    postPushedAuthorizationRequest(makeParPayload(), false);
}

void Authorization::postPushedAuthorizationRequest(const QByteArray &payload, bool retried)
{
    AtProtocolInterface::AccountData account;
    account.service = pushedAuthorizationRequestEndpoint();

    OauthPushedAuthorizationRequest *req = new OauthPushedAuthorizationRequest(this);
    connect(req, &OauthPushedAuthorizationRequest::finished, this, [=](bool success) {
        if (!req->dPopNonce().isEmpty()) {
            setDPopNonce(req->dPopNonce());
        }
        if (success) {
            if (!req->pushedAuthorizationResponse().request_uri.isEmpty()) {
                // next step
                authorization(req->pushedAuthorizationResponse().request_uri);
            } else {
                emit errorOccurred("Invalid Pushed Authorization Request",
                                   "'request_uri' is empty.");
                emit finished(false);
            }
        } else if (canRetryWithDPopNonce(req->errorCode(), req->dPopNonce(), retried)) {
            qDebug().noquote() << "Retry PAR with new DPoP nonce";
            postPushedAuthorizationRequest(payload, true);
        } else {
            emit errorOccurred(req->errorCode(), req->errorMessage());
            emit finished(false);
        }
        req->deleteLater();
    });
    req->appendRawHeader("DPoP",
                         JsonWebToken::generate(m_dPopKey, pushedAuthorizationRequestEndpoint(),
                                                "POST", dPopNonce()));
    req->setContentType("application/x-www-form-urlencoded");
    req->setAccount(account);
    req->pushedAuthorizationRequest(payload);
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

    emit madeRequestUrl(url.toString());
    // QDesktopServices::openUrl(url);
}

void Authorization::startRedirectServer()
{
    SimpleHttpServer *server = new SimpleHttpServer(this);
    QPointer<SimpleHttpServer> alive = server;
    connect(server, &SimpleHttpServer::received, this,
            [=](const QHttpServerRequest &request, bool &result, QByteArray &data,
                QByteArray &mime_type) {
                qDebug().noquote() << "received by startRedirectServer";
                qDebug().noquote() << "  " << request.url().toString();
                qDebug().noquote() << "  " << request.url().path();

                if (request.url().path() != "/tech/relog/hagoromo/oauth-callback") {
                    result = SimpleHttpServer::readFile(":/tools/oauth/" + request.url().fileName(),
                                                        data);
                    mime_type = m_MimeDb.mimeTypeForFile(request.url().fileName()).name().toUtf8();
                    qDebug().noquote() << "Other file:" << result << request.url().fileName()
                                       << ", " << mime_type;
                    return;
                }

                bool authorized = false;
                const QUrlQuery query = request.query();
                if (query.hasQueryItem("iss") && query.hasQueryItem("state")
                    && query.hasQueryItem("code")) {
                    // authorize
                    const QString iss = query.queryItemValue("iss", QUrl::FullyDecoded);
                    const QString state = query.queryItemValue("state", QUrl::FullyDecoded);
                    if (m_state.isEmpty() || state.toUtf8() != m_state) {
                        qDebug().noquote() << "Unknown state in authorization redirect :" << state;
                        emit errorOccurred("Invalid authorization response",
                                           "'state' does not match.");
                    } else if (m_issuer.isEmpty() || iss != m_issuer) {
                        qDebug().noquote() << "Unknown iss in authorization redirect :" << iss;
                        emit errorOccurred("Invalid authorization response",
                                           QString("'iss' does not match(%1).").arg(iss));
                    } else {
                        authorized = true;
                    }
                    if (authorized) {
                        m_code = query.queryItemValue("code", QUrl::FullyDecoded).toUtf8();
                        requestToken();
                    } else {
                        m_code.clear();
                        emit finished(false);
                    }
                } else {
                    // 拒否されたときなど
                    const QString error = query.queryItemValue("error", QUrl::FullyDecoded);
                    emit errorOccurred(
                            error.isEmpty() ? QStringLiteral("Invalid authorization response")
                                            : error,
                            query.queryItemValue("error_description", QUrl::FullyDecoded));
                    m_code.clear();
                    emit finished(false);
                }
                // 結果のページはどちらの場合も返す
                result = true;
                if (authorized) {
                    SimpleHttpServer::readFile(
                            ":/tech/relog/hagoromo/tools/oauth/oauth_success.html", data);
                } else {
                    SimpleHttpServer::readFile(":/tech/relog/hagoromo/tools/oauth/oauth_fail.html",
                                               data);
                }
                data.replace("%HANDLE%", m_handle.toLocal8Bit());
                qDebug().noquote() << "Result html:" << data;
                mime_type = "text/html";

                server->clearTimeout();
                // delete after 5 sec.
                QTimer::singleShot(5 * 1000, [=]() {
                    if (alive) {
                        server->deleteLater();
                    } else {
                        qDebug().noquote() << "Already deleted server";
                    }
                });
            });
    connect(server, &SimpleHttpServer::timeout, this, [=]() {
        // token取得に進んでたらfinishedは発火しない
        if (m_code.isEmpty()) {
            qDebug().noquote() << "Authorization timeout";
            emit finished(false);
        }
        server->deleteLater();
    });
    connect(server, &QObject::destroyed, this, [this]() {
        qDebug().noquote() << "Destory webserver";
        m_listenPort.clear();
    });

    server->setTimeout(redirectTimeout());
    quint16 port = server->listen(QHostAddress::LocalHost, 0);
    m_listenPort = QString::number(port);

    qDebug().noquote() << "Listen" << m_listenPort;
}

QByteArray Authorization::makeRequestTokenPayload(bool refresh)
{
    QUrlQuery query;

    if (refresh) {
        query.addQueryItem("grant_type", "refresh_token");
        query.addQueryItem("refresh_token", token().refresh_token);
        query.addQueryItem("client_id", simplyEncode(m_clientId));
    } else {
        query.addQueryItem("grant_type", "authorization_code");
        query.addQueryItem("code", m_code);
        query.addQueryItem("code_verifier", m_codeVerifier);
        query.addQueryItem("client_id", simplyEncode(m_clientId));
        query.addQueryItem("redirect_uri", simplyEncode(m_redirectUri));
    }

    return query.query(QUrl::FullyEncoded).toLocal8Bit();
}

void Authorization::requestToken(bool refresh)
{
    if (tokenEndopoint().isEmpty())
        return;
    if (!m_dPopKey.isValid()) {
        // tokenはDPoPの鍵に紐づくので、別の鍵を作って続けることはできない
        emit errorOccurred("Invalid DPoP key", "DPoP key is not set.");
        emit finished(false);
        return;
    }
    if (m_tokenRequesting) {
        // refresh tokenは使い捨てなので同時に使うと片方が失敗してセッションを失う
        // 結果は要求中のリクエストのfinished/tokenChangedで通知される
        qDebug().noquote() << "Token request is already in progress";
        return;
    }

    m_tokenRequesting = true;
    postTokenRequest(refresh, false);
}

void Authorization::postTokenRequest(bool refresh, bool retried)
{
    AtProtocolInterface::AccountData account;
    account.service = tokenEndopoint();

    OauthRequestToken *req = new OauthRequestToken(this);
    connect(req, &OauthRequestToken::finished, this, [=](bool success) {
        if (!req->dPopNonce().isEmpty()) {
            setDPopNonce(req->dPopNonce());
        }
        if (!success && canRetryWithDPopNonce(req->errorCode(), req->dPopNonce(), retried)) {
            qDebug().noquote() << "Retry token request with new DPoP nonce";
            postTokenRequest(refresh, true);
            req->deleteLater();
            return;
        }
        m_tokenRequesting = false;
        bool ret = false;
        if (success) {
            QString error_message;
            if (validateTokenResponse(req->tokenResponse(), error_message)) {
                if (m_did.isEmpty()) {
                    m_did = req->tokenResponse().sub;
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
                qDebug().noquote() << error_message;
                emit errorOccurred("Invalid token response", error_message);
            }
        } else {
            emit errorOccurred(req->errorCode(), req->errorMessage());
        }
        emit finished(ret);
        req->deleteLater();
    });
    req->appendRawHeader("DPoP",
                         JsonWebToken::generate(m_dPopKey, tokenEndopoint(), "POST", dPopNonce()));
    req->setContentType("application/x-www-form-urlencoded");
    req->setAccount(account);
    req->requestToken(makeRequestTokenPayload(refresh));
}

bool Authorization::validateTokenResponse(const AtProtocolType::OauthDefs::TokenResponse &token,
                                          QString &error_message) const
{
    // セッション開始時のDID、refreshの場合はこれまでのtokenのDIDと一致すること
    const QString expected_did = m_did.isEmpty() ? m_token.sub : m_did;

    bool ret = false;
    if (token.access_token.isEmpty()) {
        error_message = QStringLiteral("'access_token' is empty.");
    } else if (token.refresh_token.isEmpty()) {
        error_message = QStringLiteral("'refresh_token' is empty.");
    } else if (token.token_type.toLower() != "dpop") {
        error_message = QString("'token_type' is an invalid value(%1).").arg(token.token_type);
    } else if (!token.scope.split(' ', Qt::SkipEmptyParts).contains("atproto")) {
        error_message = QStringLiteral("'scope' must contain 'atproto'.");
    } else if (!token.sub.startsWith("did:")) {
        error_message = QString("'sub' is an invalid value(%1).").arg(token.sub);
    } else if (!expected_did.isEmpty() && token.sub != expected_did) {
        error_message = QString("'sub' does not match the expected DID(%1, expected %2).")
                                .arg(token.sub, expected_did);
    } else {
        ret = true;
    }
    return ret;
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
        && m_token.token_type == newToken.token_type && m_token.sub == newToken.sub
        && m_token.scope == newToken.scope)
        return;
    m_token = newToken;
    emit tokenChanged();
}

QString Authorization::clientId() const
{
    return m_clientId;
}

void Authorization::setClientId(const QString &newClientId)
{
    m_clientId = newClientId;
}

QString Authorization::dPopNonce() const
{
    return m_dPopNonce;
}

void Authorization::setDPopNonce(const QString &newDPopNonce)
{
    m_dPopNonce = newDPopNonce;
}

QByteArray Authorization::dPopPrivateKey() const
{
    return m_dPopKey.privateKeyPem();
}

bool Authorization::setDPopPrivateKey(const QByteArray &pem)
{
    return m_dPopKey.loadPrivateKeyPem(pem);
}

QByteArray Authorization::codeChallenge() const
{
    return m_codeChallenge;
}

QByteArray Authorization::codeVerifier() const
{
    return m_codeVerifier;
}
