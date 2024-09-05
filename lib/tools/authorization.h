#ifndef AUTHORIZATION_H
#define AUTHORIZATION_H

#include <QObject>
#include "atprotocol/lexicons.h"

class Authorization : public QObject
{
    Q_OBJECT
public:
    explicit Authorization(QObject *parent = nullptr);

    void reset();

    void start(const QString &pds, const QString &handle);

    void makeClientId();
    void makeCodeChallenge();
    void makeParPayload();
    void par();
    void authorization(const QString &request_uri);
    void startRedirectServer();

    void makeRequestTokenPayload();
    void requestToken();

    QString serviceEndpoint() const;
    void setServiceEndpoint(const QString &newServiceEndpoint);
    QString authorizationServer() const;
    void setAuthorizationServer(const QString &newAuthorizationServer);
    QString pushedAuthorizationRequestEndpoint() const;
    void
    setPushedAuthorizationRequestEndpoint(const QString &newPushedAuthorizationRequestEndpoint);
    QString authorizationEndpoint() const;
    void setAuthorizationEndpoint(const QString &newAuthorizationEndpoint);
    QString tokenEndopoint() const;
    void setTokenEndopoint(const QString &newTokenEndopoint);
    int redirectTimeout() const;
    void setRedirectTimeout(int newRedirectTimeout);
    AtProtocolType::OauthDefs::TokenResponse token() const;
    void setToken(const AtProtocolType::OauthDefs::TokenResponse &newToken);

    QByteArray codeVerifier() const;
    QByteArray codeChallenge() const;
    QByteArray ParPayload() const;

    QString listenPort() const;

    QByteArray state() const;

signals:
    void errorOccured(const QString &code, const QString &message);
    void serviceEndpointChanged();
    void authorizationServerChanged();
    void pushedAuthorizationRequestEndpointChanged();
    void authorizationEndpointChanged();
    void tokenEndopointChanged();
    void tokenChanged();
    void finished(bool success);
#ifdef HAGOROMO_UNIT_TEST
    void madeRequestUrl(const QString &url);
#endif

private:
    QByteArray generateRandomValues() const;
    QString simplyEncode(QString text) const;

    // server info
    void requestOauthProtectedResource();
    void requestOauthAuthorizationServer();
    bool
    validateServerMetadata(const AtProtocolType::WellKnownDefs::ServerMetadata &server_metadata,
                           QString &error_message);

    // user
    QString m_handle;
    // server info
    QString m_serviceEndpoint;
    QString m_authorizationServer;
    // server meta data
    QString m_pushedAuthorizationRequestEndpoint;
    QString m_authorizationEndpoint;
    QString m_tokenEndopoint;
    QStringList m_scopesSupported;
    //
    QString m_redirectUri;
    QString m_clientId;
    // par
    QByteArray m_codeChallenge;
    QByteArray m_codeVerifier;
    QByteArray m_state;
    QByteArray m_parPayload;
    // request token
    QByteArray m_code;
    QByteArray m_requestTokenPayload;
    AtProtocolType::OauthDefs::TokenResponse m_token;

    QString m_listenPort;
    int m_redirectTimeout;
};

#endif // AUTHORIZATION_H
