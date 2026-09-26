#ifndef AUTHORIZATION_H
#define AUTHORIZATION_H

#include <QObject>
#include <QMimeDatabase>
#include "atprotocol/lexicons.h"
#include "tools/es256.h"

class Authorization : public QObject
{
    Q_OBJECT
public:
    explicit Authorization(QObject *parent = nullptr);

    void reset();

    void start(const QString &pds, const QString &handle);

    void makeClientId();
    void makeCodeChallenge();
    QByteArray makeParPayload();
    void par();
    void authorization(const QString &request_uri);
    void startRedirectServer();

    QByteArray makeRequestTokenPayload(bool refresh);
    void requestToken(bool refresh = false);

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

    QString clientId() const;
    void setClientId(const QString &newClientId);
    QString dPopNonce() const;
    void setDPopNonce(const QString &newDPopNonce);
    // セッションに紐づくDPoPの秘密鍵(PEM)。tokenと一緒に保存し、refresh前に復元する
    QByteArray dPopPrivateKey() const;
    bool setDPopPrivateKey(const QByteArray &pem);

    QByteArray codeVerifier() const;
    QByteArray codeChallenge() const;

    QString listenPort() const;
    void setListenPort(const QString &newListenPort);
    QByteArray state() const;
    QString issuer() const;
    QString did() const;
    QString handle() const;

    // client-metadata.jsonのscopeに同じ文字列で宣言されている必要がある
    static QStringList defaultScopes();
    QStringList scopes() const;
    void setScopes(const QStringList &newScopes);
    QString plcDirectory() const;
    void setPlcDirectory(const QString &newPlcDirectory);

signals:
    void errorOccurred(const QString &code, const QString &message);
    void serviceEndpointChanged();
    void authorizationServerChanged();
    void pushedAuthorizationRequestEndpointChanged();
    void authorizationEndpointChanged();
    void tokenEndopointChanged();
    void tokenChanged();
    void finished(bool success);
    void madeRequestUrl(const QString &url); // このシグナルを受けてブラウザに飛ばすなりする

private:
    QByteArray generateRandomValues() const;
    QString simplyEncode(QString text) const;

    void postPushedAuthorizationRequest(const QByteArray &payload, bool retried);
    void postTokenRequest(bool refresh, bool retried);

    // server info
    void requestOauthProtectedResource();
    void requestOauthAuthorizationServer();
    bool
    validateServerMetadata(const AtProtocolType::WellKnownDefs::ServerMetadata &server_metadata,
                           QString &error_message);
    bool validateTokenResponse(const AtProtocolType::OauthDefs::TokenResponse &token,
                               QString &error_message) const;

    // user
    QString m_handle;
    QString m_did; // セッションで想定するアカウントのDID
    // server info
    QString m_serviceEndpoint;
    QString m_authorizationServer;
    // server meta data
    QString m_issuer;
    QString m_pushedAuthorizationRequestEndpoint;
    QString m_authorizationEndpoint;
    QString m_tokenEndopoint;
    QStringList m_scopes;
    //
    QString m_redirectUri;
    QString m_clientId;
    QString m_dPopNonce;
    Es256 m_dPopKey;
    // par
    QByteArray m_codeChallenge;
    QByteArray m_codeVerifier;
    QByteArray m_state;
    // request token
    QByteArray m_code;
    AtProtocolType::OauthDefs::TokenResponse m_token;

    QString m_plcDirectory;
    // tokenの要求中(refresh tokenは使い捨てなので同時に要求しない)
    bool m_tokenRequesting;

    QString m_listenPort;
    int m_redirectTimeout;
    QMimeDatabase m_MimeDb;
};

#endif // AUTHORIZATION_H
