#include "oauthlogin.h"
#include "tools/authorization.h"

#include <QDebug>

OAuthLogin::OAuthLogin(QObject *parent) : QObject { parent }, m_running(false) { }

OAuthLogin::~OAuthLogin()
{
    cancel();
}

void OAuthLogin::start()
{
    if (running())
        return;
    if (service().isEmpty() || identifier().isEmpty()) {
        emit errorOccurred(QStringLiteral("InvalidParameter"),
                           QStringLiteral("Service and identifier are required."));
        emit finished(false);
        return;
    }

    m_session = OAuthSession();
    setAuthorizationUrl(QString());
    setRunning(true);

    Authorization *authorization = new Authorization(this);
    m_authorization = authorization;
    connect(authorization, &Authorization::errorOccurred, this, &OAuthLogin::errorOccurred);
    connect(authorization, &Authorization::madeRequestUrl, this, [=](const QString &url) {
        setAuthorizationUrl(url);
        emit requestOpenUrl(url);
    });
    connect(authorization, &Authorization::finished, this, [=](bool success) {
        if (m_authorization != authorization) {
            // キャンセル済み
            return;
        }
        if (success) {
            m_session.handle = authorization->handle();
            m_session.service_endpoint = authorization->serviceEndpoint();
            m_session.issuer = authorization->issuer();
            m_session.token_endpoint = authorization->tokenEndopoint();
            m_session.revocation_endpoint = authorization->revocationEndpoint();
            m_session.dpop_private_key = authorization->dPopPrivateKey();
            m_session.token = authorization->token();
        }
        finish(success);
    });
    authorization->start(service(), identifier());
}

void OAuthLogin::cancel()
{
    if (m_authorization.isNull())
        return;
    // リダイレクトを待つサーバーも一緒に止まる
    Authorization *authorization = m_authorization;
    m_authorization.clear();
    authorization->disconnect(this);
    authorization->deleteLater();
    setAuthorizationUrl(QString());
    setRunning(false);
}

void OAuthLogin::finish(bool success)
{
    if (!m_authorization.isNull()) {
        // ここはAuthorizationのシグナルの中なので後で破棄する
        m_authorization->disconnect(this);
        m_authorization->deleteLater();
        m_authorization.clear();
    }
    setAuthorizationUrl(QString());
    setRunning(false);
    emit finished(success);
}

QString OAuthLogin::service() const
{
    return m_service;
}

void OAuthLogin::setService(const QString &newService)
{
    if (m_service == newService)
        return;
    m_service = newService;
    emit serviceChanged();
}

QString OAuthLogin::identifier() const
{
    return m_identifier;
}

void OAuthLogin::setIdentifier(const QString &newIdentifier)
{
    if (m_identifier == newIdentifier)
        return;
    m_identifier = newIdentifier;
    emit identifierChanged();
}

bool OAuthLogin::running() const
{
    return m_running;
}

void OAuthLogin::setRunning(bool newRunning)
{
    if (m_running == newRunning)
        return;
    m_running = newRunning;
    emit runningChanged();
}

QString OAuthLogin::authorizationUrl() const
{
    return m_authorizationUrl;
}

void OAuthLogin::setAuthorizationUrl(const QString &newAuthorizationUrl)
{
    if (m_authorizationUrl == newAuthorizationUrl)
        return;
    m_authorizationUrl = newAuthorizationUrl;
    emit authorizationUrlChanged();
}

const OAuthSession &OAuthLogin::session() const
{
    return m_session;
}

void OAuthLogin::setSession(const OAuthSession &newSession)
{
    m_session = newSession;
}
