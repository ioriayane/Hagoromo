#include <QtTest>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDesktopServices>
#include <QTemporaryFile>

#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/bn.h>

#include "tools/authorization.h"
#include "tools/jsonwebtoken.h"
#include "tools/es256.h"
#include "tools/identityresolver.h"
#include "tools/dpopsessionstore.h"
#include "tools/accountmanager.h"
#include "tools/encryption.h"
#include "common.h"
#include "extension/well-known/wellknownatprotodid.h"
#include "http/simplehttpserver.h"

#include <QHttpHeaders>

// #define AUTH_TEST_IN_PRODUCTION_ENVIRONMENT
// #define REFRESH_TEST_IN_PRODUCTION_ENVIRONMENT

// DPoPのnonceが最新でないPAR/tokenリクエストに use_dpop_nonce を返すサーバー
class DPopNonceServer : public SimpleHttpServer
{
public:
    explicit DPopNonceServer(QObject *parent = nullptr)
        : SimpleHttpServer(parent), m_rotateAlways(false), m_challengeCount(0)
    {
    }

    bool handleRequest(const QHttpServerRequest &request, QHttpServerResponder &responder) override
    {
        const QString path = request.url().path();
        if (!m_resourceNonce.isEmpty() && path.contains("/response/4/xrpc/")
            && request.headers().value("Authorization").toByteArray().startsWith("DPoP ")
            && nonceInDPop(request.headers().value("DPoP").toByteArray()) != m_resourceNonce) {
            // Resource Server(PDS)は401とWWW-Authenticateでnonceを要求する
            m_resourceChallengeCount++;
            QHttpHeaders headers;
            headers.append(QHttpHeaders::WellKnownHeader::ContentType, "application/json");
            headers.append("DPoP-Nonce", m_resourceNonce);
            headers.append("WWW-Authenticate",
                           "DPoP error=\"use_dpop_nonce\", error_description=\"Resource server "
                           "requires nonce in DPoP proof\"");
            if (m_rotateAlways) {
                m_resourceNonce =
                        QString("resource-nonce-rotated-%1").arg(m_resourceChallengeCount);
            }
            responder.write(QByteArray("{\"error\":\"use_dpop_nonce\","
                                       "\"message\":\"Resource server requires nonce in DPoP "
                                       "proof\"}"),
                            headers, QHttpServerResponder::StatusCode::Unauthorized);
            return true;
        }
        if (!m_nonce.isEmpty()
            && (path.endsWith("/oauth/par") || path.endsWith("/oauth/token")
                || path.endsWith("/oauth/revoke"))
            && nonceInDPop(request.headers().value("DPoP").toByteArray()) != m_nonce) {
            m_challengeCount++;
            QHttpHeaders headers;
            headers.append(QHttpHeaders::WellKnownHeader::ContentType, "application/json");
            headers.append("DPoP-Nonce", m_nonce);
            if (m_rotateAlways) {
                // 通知したnonceを直後に無効にして、再送も失敗させる
                m_nonce = QString("nonce-rotated-%1").arg(m_challengeCount);
            }
            responder.write(QByteArray("{\"error\":\"use_dpop_nonce\","
                                       "\"error_description\":\"Authorization server requires "
                                       "nonce in DPoP proof\"}"),
                            headers, QHttpServerResponder::StatusCode::BadRequest);
            return true;
        }
        return SimpleHttpServer::handleRequest(request, responder);
    }

    QString m_nonce;
    bool m_rotateAlways;
    int m_challengeCount;
    QString m_resourceNonce;
    int m_resourceChallengeCount = 0;

private:
    static QString nonceInDPop(const QByteArray &jwt)
    {
        const QByteArrayList parts = jwt.split('.');
        if (parts.length() != 3)
            return QString();
        const QByteArray payload = QByteArray::fromBase64(
                parts.at(1), QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
        return QJsonDocument::fromJson(payload).object().value("nonce").toString();
    }
};

// AccessAtProtocolの送信処理を直接呼ぶ
class TestAccess : public AtProtocolInterface::AccessAtProtocol
{
public:
    explicit TestAccess(QObject *parent = nullptr) : AtProtocolInterface::AccessAtProtocol(parent)
    {
    }
    void testGet()
    {
        QUrlQuery query;
        query.addQueryItem("actor", "did:plc:ipj5qejfoqu6eukvt72uhyit");
        get(QStringLiteral("xrpc/com.atproto.test.get"), query);
    }
    void testPost() { post(QStringLiteral("xrpc/com.atproto.test.post"), QByteArray("{}")); }
    void testPostWithImage(const QString &path)
    {
        postWithImage(QStringLiteral("xrpc/com.atproto.test.upload"), path);
    }

private:
    virtual bool parseJson(bool success, const QString reply_json)
    {
        Q_UNUSED(reply_json)
        return success;
    }
};

class oauth_test : public QObject
{
    Q_OBJECT

public:
    oauth_test();
    ~oauth_test();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_oauth_server();
#ifndef AUTH_TEST_IN_PRODUCTION_ENVIRONMENT
    void test_oauth_process();
    void test_oauth();
    void test_oauth_dpop_nonce_retry_limit();
    void test_oauth_token_validation();
    void test_oauth_token_request_lock();
    void test_identity_resolver();
    void test_client_metadata();
    void test_identity_resolver_online();
    void test_oauth_par_online();
    void test_well_known_atproto_did();
    void test_access_password();
    void test_access_oauth();
    void test_account_manager_oauth();
    void test_account_manager_legacy_encryption();
    void test_oauth_revoke();
    void test_no_secret_in_logs();
    void test_jwt();
    void test_es256();
#endif

private:
    DPopNonceServer m_server;
    quint16 m_listenPort;

    // サーバーが受け付けたPAR/tokenリクエストのDPoP
    QList<QJsonObject> m_dPopHeaders;
    QList<QJsonObject> m_dPopPayloads;
    // サーバーが受け付けたPARのリクエストボディ
    QList<QByteArray> m_parBodies;
    // サーバーが受け付けた失効のリクエストボディ
    QList<QByteArray> m_revokeBodies;
    // サーバーが受け付けたResource Serverへのリクエストのヘッダー
    QList<QByteArray> m_resourceAuthorizations;
    QList<QByteArray> m_resourceDPops;

    void test_get(const QString &url, const QByteArray &except_data);
    void verify_jwt(const QByteArray &jwt, EVP_PKEY *pkey);
    static QJsonObject decode_jwt_part(const QByteArray &part);
    static QByteArray generate_private_key_pem();
};

oauth_test::oauth_test()
{
    QCoreApplication::setOrganizationName(QStringLiteral("relog"));
    // account.jsonを保存するので、並列で動く他のテストとは別のフォルダにする
    QCoreApplication::setApplicationName(QStringLiteral("Hagoromo_unittest_oauth"));

    m_listenPort = m_server.listen(QHostAddress::LocalHost, 0);
    connect(&m_server, &SimpleHttpServer::received, this,
            [=](const QHttpServerRequest &request, bool &result, QByteArray &data,
                QByteArray &mime_type) {
                //
                qDebug().noquote() << request.url();
                QString path = SimpleHttpServer::convertResoucePath(request.url());
                qDebug().noquote() << " res path =" << path;

                if (path.contains("/response/plc/")) {
                    // Windowsのファイル名に':'は使えないので置き換える
                    path.replace(path.lastIndexOf('/') + 1, path.length(),
                                 QString(path.mid(path.lastIndexOf('/') + 1)).replace(':', '_'));
                }
                if (path.endsWith("/oauth/par")) {
                    m_parBodies.append(request.body());
                }
                if (path.contains("/response/4/xrpc/")) {
                    m_resourceAuthorizations.append(
                            request.headers().value("Authorization").toByteArray());
                    const QByteArray dpop = request.headers().value("DPoP").toByteArray();
                    if (!dpop.isEmpty()) {
                        verify_jwt(dpop, nullptr);
                    }
                    m_resourceDPops.append(dpop);
                }
                if (path.endsWith("/oauth/revoke")) {
                    m_revokeBodies.append(request.body());
                }
                if (path.endsWith("/oauth/par") || path.endsWith("/oauth/token")
                    || path.endsWith("/oauth/revoke")) {
                    qDebug().noquote() << "Verify jwt";
                    bool exist = false;
                    for (const auto &header : request.headers().toListOfPairs()) {
                        if (header.first.toLower() == "dpop") {
                            verify_jwt(header.second, nullptr);
                            const QByteArrayList parts = header.second.split('.');
                            if (parts.length() == 3) {
                                m_dPopHeaders.append(decode_jwt_part(parts.at(0)));
                                m_dPopPayloads.append(decode_jwt_part(parts.at(1)));
                            }
                            exist = true;
                            break;
                        }
                    }
                    QVERIFY(exist);
                }

                if (!QFile::exists(path)) {
                    result = false;
                } else {
                    mime_type = path.endsWith("/atproto-did") ? "text/plain" : "application/json";
                    result = SimpleHttpServer::readFile(path, data);
                    data.replace("{{SERVER_PORT_NO}}", QString::number(m_listenPort).toLocal8Bit());
                    qDebug().noquote() << " result =" << result;
                }
            });
}

oauth_test::~oauth_test() { }

void oauth_test::initTestCase() { }

void oauth_test::cleanupTestCase() { }

void oauth_test::test_oauth_server()
{
    Authorization oauth;
#ifdef AUTH_TEST_IN_PRODUCTION_ENVIRONMENT
    QSignalSpy spy_error(&oauth, SIGNAL(errorOccurred(const QString &, const QString &)));
    {
        // ID解決(DNS/HTTPS/PLC)、メタデータ取得、PAR(nonceの再送を含む)まで数秒かかる
        QSignalSpy spy(&oauth, SIGNAL(madeRequestUrl(const QString &)));
        QSignalSpy spy_finished(&oauth, SIGNAL(finished(bool)));
        oauth.start("https://bsky.social", "ioriayane.bsky.social");
        for (int i = 0; i < 30 && spy.isEmpty() && spy_finished.isEmpty(); i++) {
            spy.wait(1000);
        }
        for (const auto &error : spy_error) {
            qDebug().noquote() << "error :" << error.at(0).toString() << error.at(1).toString();
        }
        QCOMPARE(spy.count(), 1);
        QList<QVariant> arguments = spy.takeFirst();
        QString request_url = arguments.at(0).toString();
        qDebug().noquote() << "request url:" << request_url;
        QDesktopServices::openUrl(request_url);
    }
    {
        // ブラウザでログインする
        QSignalSpy spy(&oauth, SIGNAL(finished(bool)));
        spy.wait(5 * 60 * 1000);
        for (const auto &error : spy_error) {
            qDebug().noquote() << "error :" << error.at(0).toString() << error.at(1).toString();
        }
        QCOMPARE(spy.count(), 1);
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }
    QCOMPARE(oauth.token().sub, oauth.did());
    QVERIFY(oauth.token().scope.split(' ').contains("atproto"));
    qDebug().noquote() << "granted scope:" << oauth.token().scope;
    {
        // 同じセッション(DPoPの鍵)でrefreshできること
        const QString old_refresh_token = oauth.token().refresh_token;
        QSignalSpy spy(&oauth, SIGNAL(finished(bool)));
        oauth.requestToken(true);
        spy.wait(30 * 1000);
        for (const auto &error : spy_error) {
            qDebug().noquote() << "error :" << error.at(0).toString() << error.at(1).toString();
        }
        QCOMPARE(spy.count(), 1);
        QVERIFY(spy.takeFirst().at(0).toBool());
        QCOMPARE(oauth.token().sub, oauth.did());
        // refresh tokenは使い捨てで更新される
        QVERIFY(oauth.token().refresh_token != old_refresh_token);
    }
    {
        // 失効させると、そのrefresh tokenではもう更新できない
        QSignalSpy spy_revoke(&oauth, SIGNAL(revokeFinished(bool)));
        oauth.revokeToken();
        spy_revoke.wait(30 * 1000);
        QCOMPARE(spy_revoke.count(), 1);
        QVERIFY(spy_revoke.takeFirst().at(0).toBool());

        QSignalSpy spy(&oauth, SIGNAL(finished(bool)));
        oauth.requestToken(true);
        spy.wait(30 * 1000);
        QCOMPARE(spy.count(), 1);
        QVERIFY(!spy.takeFirst().at(0).toBool());
    }
#elif defined(REFRESH_TEST_IN_PRODUCTION_ENVIRONMENT)
    AtProtocolType::OauthDefs::TokenResponse token;
    token.refresh_token = "ref-xxxxxxxxxxxxx"; // ここに実際のリフレッシュトークンを設定する
    oauth.setToken(token);
    oauth.setTokenEndopoint("https://bsky.social/oauth/token");
    oauth.setDPopNonce("8mo0kjo");
    oauth.setListenPort("65073");
    oauth.makeClientId();
    {
        QSignalSpy spy(&oauth, SIGNAL(finished(bool)));
        oauth.requestToken(true);
        spy.wait();
        QCOMPARE(spy.count(), 1);
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }
#endif
}

#ifndef AUTH_TEST_IN_PRODUCTION_ENVIRONMENT
void oauth_test::test_oauth_process()
{
    QString code_challenge;

    const uint8_t base[] = { 116, 24,  223, 180, 151, 153, 224, 37,  79,  250, 96,
                             125, 216, 173, 187, 186, 22,  212, 37,  77,  105, 214,
                             191, 240, 91,  88,  5,   88,  83,  132, 141, 121 };
    QByteArray base_ba; //(QByteArray::fromRawData(static_cast<const char *>(base), sizeof(base)));
    for (int i = 0; i < sizeof(base); i++) {
        base_ba.append(base[i]);
    }
    qDebug() << "codeVerifier" << sizeof(base) << base_ba.size()
             << base_ba.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);

    QString msg;
    QByteArray sha256 = QCryptographicHash::hash(
            base_ba.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals),
            QCryptographicHash::Sha256);
    for (const auto s : sha256) {
        msg += QString::number(static_cast<unsigned char>(s)) + ", ";
    }
    qDebug() << msg;
    qDebug() << "codeChallenge"
             << sha256.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);

    Authorization oauth;
    oauth.makeCodeChallenge();
    qDebug() << "codeChallenge" << oauth.codeChallenge();
    qDebug() << "codeVerifier" << oauth.codeVerifier();

    // QVERIFY(oauth.codeChallenge()
    //         == sha256.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
    // QVERIFY(oauth.codeVerifier()
    //         == base_ba.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
}

void oauth_test::test_oauth()
{
    // response/1 : pds
    // response/2 : entry-way

    Authorization oauth;
    oauth.setRedirectTimeout(20);
    oauth.setPlcDirectory(QString("http://127.0.0.1:%1/response/plc").arg(m_listenPort));

    // DNS/HTTPSでは解決できないハンドルにして、pdsのresolveHandleで解決させる
    QString pds = QString("http://127.0.0.1:%1/response/2").arg(m_listenPort);
    QString handle = "@IoriAyane.test";

    m_server.m_nonce = "nonce-par";
    m_server.m_rotateAlways = false;
    m_server.m_challengeCount = 0;
    m_dPopHeaders.clear();
    m_dPopPayloads.clear();
    m_parBodies.clear();

    oauth.reset();
    // 以前のセッションの鍵は使われず、新しい鍵が作られること
    const QByteArray old_private_key = generate_private_key_pem();
    QVERIFY(oauth.setDPopPrivateKey(old_private_key));
    {
        QSignalSpy spy(&oauth, SIGNAL(serviceEndpointChanged()));
        oauth.start(pds, handle);
        spy.wait(20 * 1000);
        QCOMPARE(spy.count(), 1);
    }
    QCOMPARE(oauth.serviceEndpoint(), QString("http://127.0.0.1:%1/response/1").arg(m_listenPort));
    QCOMPARE(oauth.handle(), QString("ioriayane.test"));

    {
        QSignalSpy spy(&oauth, SIGNAL(authorizationServerChanged()));
        spy.wait();
        QCOMPARE(spy.count(), 1);
    }
    QVERIFY(oauth.authorizationServer()
            == QString("http://127.0.0.1:%1/response/2").arg(m_listenPort));

    {
        QSignalSpy spy(&oauth, SIGNAL(pushedAuthorizationRequestEndpointChanged()));
        spy.wait();
        QCOMPARE(spy.count(), 1);
    }
    QVERIFY(oauth.pushedAuthorizationRequestEndpoint()
            == QString("http://127.0.0.1:%1/response/2/oauth/par").arg(m_listenPort));
    QVERIFY(oauth.authorizationEndpoint()
            == QString("http://127.0.0.1:%1/response/2/oauth/authorize").arg(m_listenPort));
    QCOMPARE(oauth.issuer(), QString("http://127.0.0.1:%1").arg(m_listenPort));
    QCOMPARE(oauth.did(), QString("did:plc:ipj5qejfoqu6eukvt72uhyit"));

    //
    QString request_url;
    {
        QSignalSpy spy(&oauth, SIGNAL(madeRequestUrl(const QString &)));
        spy.wait();
        QCOMPARE(spy.count(), 1);
        QList<QVariant> arguments = spy.takeFirst();
        request_url = arguments.at(0).toString();
        QCOMPARE(request_url,
                 (QStringLiteral("http://127.0.0.1:") + QString::number(m_listenPort)
                  + QStringLiteral("/response/2/oauth/"
                                   "authorize?client_id=https%3A%2F%2Foauth.hagoromo.relog.tech%"
                                   "2Foauth-client-metadata.json&request_uri=urn%3Aietf%"
                                   "3Aparams%3Aoauth%3Arequest_uri%3Areq-"
                                   "05650c01604941dc674f0af9cb032aca")));
    }
    // PARのパラメータ
    QCOMPARE(m_parBodies.length(), 1);
    {
        const QUrlQuery par_query(QString::fromUtf8(m_parBodies.first()));
        QCOMPARE(par_query.queryItemValue("scope", QUrl::FullyDecoded),
                 Authorization::defaultScopes().join(" "));
        QCOMPARE(par_query.queryItemValue("login_hint", QUrl::FullyDecoded),
                 QString("ioriayane.test"));
        QVERIFY(!par_query.queryItemValue("scope", QUrl::FullyDecoded).contains("transition:"));
    }
    // 1回目のPARはnonceなしで拒否され、受け取ったnonceで再送される
    QCOMPARE(m_server.m_challengeCount, 1);
    QCOMPARE(oauth.dPopNonce(), QString("nonce-par"));

    {
        // ブラウザで認証ができないのでタイムアウトしてくるのを確認
        QSignalSpy spy(&oauth, SIGNAL(finished(bool)));
        spy.wait(20 * 1000);
        QCOMPARE(spy.count(), 1);
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(!arguments.at(0).toBool());
    }

    // stateはPKCEの値と独立していること
    QVERIFY(!oauth.state().isEmpty());
    QVERIFY(oauth.state() != oauth.codeChallenge());

    // iss/stateが一致しないリダイレクトは拒否される
    {
        const QString valid_iss = oauth.issuer();
        const QString valid_state = QString::fromUtf8(oauth.state());
        const QList<QPair<QString, QString>> invalid_params = QList<QPair<QString, QString>>()
                << qMakePair(QString("iss-hogehoge"), valid_state)
                << qMakePair(QString("%1/").arg(valid_iss), valid_state)
                << qMakePair(valid_iss, QString("state-hogehoge"));
        for (const auto &param : invalid_params) {
            QUrl invalid_url("http://127.0.0.1/tech/relog/hagoromo/oauth-callback");
            QUrlQuery invalid_query;
            invalid_query.addQueryItem("iss", param.first);
            invalid_query.addQueryItem("state", param.second);
            invalid_query.addQueryItem("code", "code-hogehoge");
            invalid_url.setQuery(invalid_query);

            QSignalSpy spy_error(&oauth, SIGNAL(errorOccurred(const QString &, const QString &)));
            QSignalSpy spy(&oauth, SIGNAL(finished(bool)));
            oauth.startRedirectServer();
            invalid_url.setPort(oauth.listenPort().toInt());
            test_get(invalid_url.toString(), QByteArray());
            spy.wait();
            QCOMPARE(spy.count(), 1);
            QVERIFY(!spy.takeFirst().at(0).toBool());
            QCOMPARE(spy_error.count(), 1);
            QCOMPARE(spy_error.takeFirst().at(0).toString(),
                     QString("Invalid authorization response"));
        }
    }

    // ブラウザに認証しにいくURLからリダイレクトURLを取り出す
    QUrl redirect_url;
    {
        redirect_url = "http://127.0.0.1/tech/relog/hagoromo/oauth-callback";
        QUrlQuery redirect_query;
        redirect_query.addQueryItem("iss", oauth.issuer());
        redirect_query.addQueryItem("state", oauth.state());
        redirect_query.addQueryItem("code", "code-hogehoge");
        redirect_url.setQuery(redirect_query);
        qDebug().noquote() << "extract to " << redirect_url;
    }
    // tokenリクエストまでにnonceがローテーションされたケース
    m_server.m_nonce = "nonce-token";
    // 認証終了したていで続き
    {
        QSignalSpy spy(&oauth, SIGNAL(tokenChanged()));
        oauth.startRedirectServer();
        redirect_url.setPort(oauth.listenPort().toInt());
        qDebug().noquote() << "port updated " << redirect_url;
        test_get(redirect_url.toString(), QByteArray()); // ブラウザへのアクセスを模擬
        spy.wait();
        QCOMPARE(spy.count(), 1);
    }

    QCOMPARE(oauth.token().access_token, "access token");
    QCOMPARE(oauth.token().token_type, "DPoP");
    QCOMPARE(oauth.token().refresh_token, "refresh token");
    QCOMPARE(oauth.token().expires_in, 2677);
    QCOMPARE(oauth.token().sub, "did:plc:ipj5qejfoqu6eukvt72uhyit");
    QCOMPARE(m_server.m_challengeCount, 2);
    QCOMPARE(oauth.dPopNonce(), QString("nonce-token"));

    // PARとtokenは同じセッションの鍵で署名されていること
    const QByteArray private_key = oauth.dPopPrivateKey();
    QVERIFY(!private_key.isEmpty());
    QVERIFY(private_key != old_private_key);
    Es256 session_key;
    QVERIFY(session_key.loadPrivateKeyPem(private_key));
    QByteArray x_coord;
    QByteArray y_coord;
    QVERIFY(session_key.getAffineCoordinates(x_coord, y_coord));

    const QStringList htu_list = QStringList()
            << QString("http://127.0.0.1:%1/response/2/oauth/par").arg(m_listenPort)
            << QString("http://127.0.0.1:%1/response/2/oauth/token").arg(m_listenPort);
    QCOMPARE(m_dPopHeaders.length(), 2);
    QCOMPARE(m_dPopPayloads.length(), 2);
    for (int i = 0; i < m_dPopHeaders.length(); i++) {
        const QJsonObject jwk = m_dPopHeaders.at(i).value("jwk").toObject();
        QCOMPARE(jwk.value("x").toString(), QString::fromUtf8(x_coord));
        QCOMPARE(jwk.value("y").toString(), QString::fromUtf8(y_coord));
        QVERIFY(!jwk.contains("d"));

        const QJsonObject payload = m_dPopPayloads.at(i);
        QCOMPARE(payload.value("htm").toString(), QString("POST"));
        QCOMPARE(payload.value("htu").toString(), htu_list.at(i));
        QVERIFY(!payload.contains("ath"));
    }
    QVERIFY(m_dPopPayloads.at(0).value("jti") != m_dPopPayloads.at(1).value("jti"));
}

void oauth_test::test_oauth_dpop_nonce_retry_limit()
{
    // 再送してもnonceエラーになる場合は1回で諦める
    m_server.m_nonce = "nonce-1";
    m_server.m_rotateAlways = true;
    m_server.m_challengeCount = 0;

    Authorization oauth;
    AtProtocolType::OauthDefs::TokenResponse token;
    token.refresh_token = "refresh token";
    oauth.setToken(token);
    QVERIFY(oauth.setDPopPrivateKey(generate_private_key_pem()));
    oauth.setTokenEndopoint(
            QString("http://127.0.0.1:%1/response/2/oauth/token").arg(m_listenPort));
    oauth.makeClientId();
    {
        QSignalSpy spy_error(&oauth, SIGNAL(errorOccurred(const QString &, const QString &)));
        QSignalSpy spy(&oauth, SIGNAL(finished(bool)));
        oauth.requestToken(true);
        spy.wait();
        QCOMPARE(spy.count(), 1);
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(!arguments.at(0).toBool());
        QCOMPARE(spy_error.count(), 1);
        QCOMPARE(spy_error.takeFirst().at(0).toString(), QString("use_dpop_nonce"));
    }
    QCOMPARE(m_server.m_challengeCount, 2);

    m_server.m_nonce.clear();
    m_server.m_rotateAlways = false;
}

void oauth_test::test_oauth_token_validation()
{
    const QString expected_did = "did:plc:ipj5qejfoqu6eukvt72uhyit";
    const QList<QPair<QString, bool>> cases = QList<QPair<QString, bool>>()
            << qMakePair(QString("token"), true) << qMakePair(QString("token_invalid_sub"), false)
            << qMakePair(QString("token_no_atproto_scope"), false);

    {
        // DPoPの鍵がない場合はtokenを要求しない
        Authorization oauth;
        oauth.setTokenEndopoint(
                QString("http://127.0.0.1:%1/response/2/oauth/token").arg(m_listenPort));
        QSignalSpy spy_error(&oauth, SIGNAL(errorOccurred(const QString &, const QString &)));
        QSignalSpy spy(&oauth, SIGNAL(finished(bool)));
        oauth.requestToken(true);
        QCOMPARE(spy.count(), 1);
        QVERIFY(!spy.takeFirst().at(0).toBool());
        QCOMPARE(spy_error.count(), 1);
        QCOMPARE(spy_error.takeFirst().at(0).toString(), QString("Invalid DPoP key"));
    }

    for (const auto &item : cases) {
        qDebug().noquote() << "token response :" << item.first;
        Authorization oauth;
        AtProtocolType::OauthDefs::TokenResponse token;
        token.refresh_token = "refresh token";
        token.sub = expected_did;
        oauth.setToken(token);
        QVERIFY(oauth.setDPopPrivateKey(generate_private_key_pem()));
        oauth.setTokenEndopoint(QString("http://127.0.0.1:%1/response/2/oauth/%2")
                                        .arg(m_listenPort)
                                        .arg(item.first));
        oauth.makeClientId();

        QSignalSpy spy_error(&oauth, SIGNAL(errorOccurred(const QString &, const QString &)));
        QSignalSpy spy(&oauth, SIGNAL(finished(bool)));
        oauth.requestToken(true);
        spy.wait();
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.takeFirst().at(0).toBool(), item.second);
        if (item.second) {
            QCOMPARE(spy_error.count(), 0);
            QCOMPARE(oauth.token().sub, expected_did);
            QCOMPARE(oauth.token().access_token, QString("access token"));
        } else {
            QCOMPARE(spy_error.count(), 1);
            QCOMPARE(spy_error.takeFirst().at(0).toString(), QString("Invalid token response"));
            // 不正な応答でtokenは更新されない
            QCOMPARE(oauth.token().access_token, QString());
        }
    }
}

void oauth_test::test_oauth_token_request_lock()
{
    // refresh中に再度refreshしても、tokenのリクエストは1回だけ
    m_dPopPayloads.clear();

    Authorization oauth;
    AtProtocolType::OauthDefs::TokenResponse token;
    token.refresh_token = "refresh token";
    token.sub = "did:plc:ipj5qejfoqu6eukvt72uhyit";
    oauth.setToken(token);
    QVERIFY(oauth.setDPopPrivateKey(generate_private_key_pem()));
    oauth.setTokenEndopoint(
            QString("http://127.0.0.1:%1/response/2/oauth/token").arg(m_listenPort));
    oauth.makeClientId();

    QSignalSpy spy_token(&oauth, SIGNAL(tokenChanged()));
    QSignalSpy spy(&oauth, SIGNAL(finished(bool)));
    oauth.requestToken(true);
    oauth.requestToken(true);
    spy.wait();
    QTest::qWait(500);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.takeFirst().at(0).toBool());
    QCOMPARE(spy_token.count(), 1);
    QCOMPARE(m_dPopPayloads.length(), 1);

    // 終わった後は再度要求できる
    oauth.requestToken(true);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    QCOMPARE(m_dPopPayloads.length(), 2);
}

void oauth_test::test_identity_resolver()
{
    const QString plc = QString("http://127.0.0.1:%1/response/plc").arg(m_listenPort);
    const QString service = QString("http://127.0.0.1:%1/response/2").arg(m_listenPort);
    const QString pds = QString("http://127.0.0.1:%1/response/1").arg(m_listenPort);
    const QString did = "did:plc:ipj5qejfoqu6eukvt72uhyit";

    struct TestCase
    {
        QString identifier;
        QString service;
        bool success;
        QString did;
        QString handle;
        QString error_code;
    };
    const QList<TestCase> cases = QList<TestCase>()
            // ハンドルから
            << TestCase { "ioriayane.test", service, true, did, "ioriayane.test", QString() }
            // DIDから(ハンドルも双方向で検証される)
            << TestCase { did, service, true, did, "ioriayane.test", QString() }
            // DIDから(ハンドルを検証できなくてもDIDは使える)
            << TestCase { did, QString(), true, did, QString(), QString() }
            // DIDドキュメントが別のハンドルを主張している
            << TestCase { "other.test", service, false, QString(), QString(), "Invalid identity" }
            // ハンドルを解決できない
            << TestCase { "ioriayane.test", QString(), false,
                          QString(),        QString(), "Failed to resolve handle" } // PDSがない
            << TestCase { "did:plc:nopds", service, false, QString(), QString(), "Invalid identity" }
            // DIDドキュメントのidが違う
            << TestCase { "did:plc:mismatch", service,   false,
                          QString(),          QString(), "Invalid identity" }
            // DIDドキュメントがない
            << TestCase { "did:plc:notfound", service,   false,
                          QString(),          QString(), "Failed to resolve DID" } // 不正な書式
            << TestCase { "invalid_handle", service,   false,
                          QString(),        QString(), "Invalid identifier" }
            << TestCase { "hagoromo.invalid", service,   false,
                          QString(),          QString(), "Invalid identifier" }
            << TestCase {
                   "did:key:hoge", service, false, QString(), QString(), "Invalid identifier"
               };

    for (const auto &item : cases) {
        qDebug().noquote() << "resolve :" << item.identifier << item.service;
        IdentityResolver resolver;
        resolver.setPlcDirectory(plc);
        resolver.setHandleResolutionService(item.service);
        resolver.setDnsTimeout(3000);

        QSignalSpy spy_error(&resolver, SIGNAL(errorOccurred(const QString &, const QString &)));
        QSignalSpy spy(&resolver, SIGNAL(finished(bool)));
        resolver.resolve(item.identifier);
        if (spy.isEmpty()) {
            spy.wait(20 * 1000);
        }
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.takeFirst().at(0).toBool(), item.success);
        QCOMPARE(resolver.did(), item.did);
        QCOMPARE(resolver.handle(), item.handle);
        if (item.success) {
            QCOMPARE(resolver.pdsEndpoint(), pds);
            QCOMPARE(spy_error.count(), 0);
        } else {
            QVERIFY(resolver.pdsEndpoint().isEmpty());
            QCOMPARE(spy_error.count(), 1);
            QCOMPARE(spy_error.takeFirst().at(0).toString(), item.error_code);
        }
    }

    QCOMPARE(IdentityResolver::normalizeHandle(" IoriAyane.Test "), QString("ioriayane.test"));
    QVERIFY(IdentityResolver::isValidHandle("ioriayane.bsky.social"));
    QVERIFY(IdentityResolver::isValidHandle("xn--ls8h.test"));
    QVERIFY(!IdentityResolver::isValidHandle("bsky"));
    QVERIFY(!IdentityResolver::isValidHandle("-hoge.bsky.social"));
    QVERIFY(!IdentityResolver::isValidHandle("hoge.123"));
    QVERIFY(!IdentityResolver::isValidHandle("hoge.local"));
    QVERIFY(IdentityResolver::isValidDid("did:plc:ipj5qejfoqu6eukvt72uhyit"));
    QVERIFY(IdentityResolver::isValidDid("did:web:example.com"));
    QVERIFY(!IdentityResolver::isValidDid("did:key:zQ3sh"));
    QVERIFY(!IdentityResolver::isValidDid("did:plc:"));
}

void oauth_test::test_identity_resolver_online()
{
    if (qEnvironmentVariable("HAGOROMO_ONLINE_TEST") != "1") {
        QSKIP("Set HAGOROMO_ONLINE_TEST=1 to resolve real identities.");
    }
    // DNS TXT / HTTPS well-knownで解決できるハンドル(フォールバックのサービスなし)
    const QStringList handles = QStringList() << "ioriayane.relog.tech"
                                              << "ioriayane.bsky.social";
    for (const auto &handle : handles) {
        qDebug().noquote() << "resolve :" << handle;
        IdentityResolver resolver;
        QSignalSpy spy(&resolver, SIGNAL(finished(bool)));
        resolver.resolve(handle);
        spy.wait(20 * 1000);
        QCOMPARE(spy.count(), 1);
        QVERIFY(spy.takeFirst().at(0).toBool());
        QVERIFY(resolver.did().startsWith("did:plc:"));
        QCOMPARE(resolver.handle(), handle);
        QVERIFY(resolver.pdsEndpoint().startsWith("https://"));
        qDebug().noquote() << "  " << resolver.did() << resolver.pdsEndpoint();
    }
}

void oauth_test::test_client_metadata()
{
    // サーバーに置くoauth-client-metadata.jsonの原本とアプリの設定が一致していること
    QByteArray data;
    QVERIFY(SimpleHttpServer::readFile(":/oauth-client-metadata.json", data));
    const QJsonObject metadata = QJsonDocument::fromJson(data).object();
    QVERIFY(!metadata.isEmpty());

    // 要求するscopeは文字列で完全一致している必要がある
    const QStringList declared_scopes = metadata.value("scope").toString().split(' ');
    for (const auto &scope : Authorization::defaultScopes()) {
        QVERIFY2(declared_scopes.contains(scope), qPrintable(scope));
    }
    QCOMPARE(metadata.value("scope").toString(), Authorization::defaultScopes().join(" "));

    Authorization oauth;
    oauth.makeClientId();
    QCOMPARE(metadata.value("client_id").toString(), oauth.clientId());
    QCOMPARE(metadata.value("application_type").toString(), QString("native"));
    QCOMPARE(metadata.value("token_endpoint_auth_method").toString(), QString("none"));
    QVERIFY(metadata.value("dpop_bound_access_tokens").toBool());
    // loopbackはポートを照合しない
    QVERIFY(metadata.value("redirect_uris")
                    .toArray()
                    .contains(QJsonValue("http://127.0.0.1/tech/relog/hagoromo/oauth-callback")));
}

void oauth_test::test_oauth_par_online()
{
    if (qEnvironmentVariable("HAGOROMO_ONLINE_TEST") != "1") {
        QSKIP("Set HAGOROMO_ONLINE_TEST=1 to access the production servers.");
    }
    // 本番環境でPARまで進めて、ブラウザで開く認可URLを作れること(ログイン操作は不要)
    Authorization oauth;
    oauth.setRedirectTimeout(10);
    QSignalSpy spy_error(&oauth, SIGNAL(errorOccurred(const QString &, const QString &)));
    QSignalSpy spy_url(&oauth, SIGNAL(madeRequestUrl(const QString &)));
    QSignalSpy spy_finished(&oauth, SIGNAL(finished(bool)));
    oauth.start("https://bsky.social", "ioriayane.bsky.social");
    for (int i = 0; i < 30 && spy_url.isEmpty() && spy_finished.isEmpty(); i++) {
        spy_url.wait(1000);
    }
    for (const auto &error : spy_error) {
        qDebug().noquote() << "error :" << error.at(0).toString() << error.at(1).toString();
    }
    QCOMPARE(spy_error.count(), 0);
    QCOMPARE(spy_url.count(), 1);

    const QUrl url(spy_url.takeFirst().at(0).toString());
    const QUrlQuery query(url.query());
    QCOMPARE(url.host(), QUrl(oauth.authorizationEndpoint()).host());
    QCOMPARE(query.queryItemValue("client_id", QUrl::FullyDecoded), oauth.clientId());
    QVERIFY(query.queryItemValue("request_uri", QUrl::FullyDecoded)
                    .startsWith("urn:ietf:params:oauth:request_uri:"));
    QCOMPARE(oauth.did(), QString("did:plc:l4fsx4ujos7uw7n4ijq2ulgs"));
    QVERIFY(!oauth.dPopNonce().isEmpty());
}

void oauth_test::test_well_known_atproto_did()
{
    AtProtocolInterface::AccountData account;
    account.service = QString("http://127.0.0.1:%1/response/3").arg(m_listenPort);

    AtProtocolInterface::WellKnownAtprotoDid well_known;
    well_known.setAccount(account);
    QSignalSpy spy(&well_known, SIGNAL(finished(bool)));
    well_known.atprotoDid();
    spy.wait();
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.takeFirst().at(0).toBool());
    QCOMPARE(well_known.did(), QString("did:plc:ipj5qejfoqu6eukvt72uhyit"));
}

void oauth_test::test_access_password()
{
    // パスワード方式は従来どおりBearerでDPoPを付けない
    m_server.m_resourceNonce = "resource-nonce";
    m_server.m_resourceChallengeCount = 0;
    m_resourceAuthorizations.clear();
    m_resourceDPops.clear();

    AtProtocolInterface::AccountData account;
    account.uuid = "uuid-password";
    account.service = QString("http://127.0.0.1:%1/response/4").arg(m_listenPort);
    account.did = "did:plc:ipj5qejfoqu6eukvt72uhyit";
    account.accessJwt = "access token";
    QCOMPARE(account.auth_type, AtProtocolInterface::AuthType::Password);

    QTemporaryFile image(QDir::tempPath() + "/hagoromo_XXXXXX.png");
    QVERIFY(image.open());
    image.write(QByteArray("dummy image"));
    image.close();

    for (int i = 0; i < 3; i++) {
        TestAccess access;
        access.setAccount(account);
        QSignalSpy spy(&access, SIGNAL(finished(bool)));
        if (i == 0) {
            access.testGet();
        } else if (i == 1) {
            access.testPost();
        } else {
            access.testPostWithImage(image.fileName());
        }
        spy.wait();
        QCOMPARE(spy.count(), 1);
        QVERIFY(spy.takeFirst().at(0).toBool());
    }
    QCOMPARE(m_server.m_resourceChallengeCount, 0);
    QCOMPARE(m_resourceAuthorizations.length(), 3);
    for (int i = 0; i < 3; i++) {
        QCOMPARE(m_resourceAuthorizations.at(i), QByteArray("Bearer access token"));
        QVERIFY(m_resourceDPops.at(i).isEmpty());
    }
    m_server.m_resourceNonce.clear();
}

void oauth_test::test_access_oauth()
{
    const QString uuid = "uuid-oauth";
    const QString access_token = "access token";
    const QString base = QString("http://127.0.0.1:%1/response/4/xrpc/").arg(m_listenPort);

    Es256 key;
    QVERIFY(key.generateKey());
    QByteArray x_coord;
    QByteArray y_coord;
    QVERIFY(key.getAffineCoordinates(x_coord, y_coord));
    DPopSessionStore *store = DPopSessionStore::getInstance();
    QVERIFY(!store->setPrivateKey(QString(), key.privateKeyPem()));
    QVERIFY(!store->setPrivateKey(uuid, QByteArray("invalid")));
    QVERIFY(!store->hasSession(uuid));
    QVERIFY(store->setPrivateKey(uuid, key.privateKeyPem()));
    QVERIFY(store->hasSession(uuid));

    AtProtocolInterface::AccountData account;
    account.uuid = uuid;
    account.auth_type = AtProtocolInterface::AuthType::OAuth;
    account.service = QString("http://127.0.0.1:%1/response/4").arg(m_listenPort);
    account.did = "did:plc:ipj5qejfoqu6eukvt72uhyit";
    account.accessJwt = access_token;

    QTemporaryFile image(QDir::tempPath() + "/hagoromo_XXXXXX.png");
    QVERIFY(image.open());
    image.write(QByteArray("dummy image"));
    image.close();

    const QString ath = QString::fromUtf8(
            QCryptographicHash::hash(access_token.toUtf8(), QCryptographicHash::Sha256)
                    .toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
    struct TestCase
    {
        int kind; // 0:get 1:post 2:postWithImage
        QString server_nonce;
        QString htm;
        QString htu;
        int challenge_count; // このリクエストでのnonce要求の回数
    };
    const QList<TestCase> cases = QList<TestCase>()
            // 最初はnonceがないので1回要求されて再送
            << TestCase { 0, "resource-nonce-1", "GET", base + "com.atproto.test.get", 1 }
            // 覚えたnonceをそのまま使う
            << TestCase { 1, "resource-nonce-1", "POST", base + "com.atproto.test.post", 0 }
            // nonceがローテーションされたら再送
            << TestCase { 2, "resource-nonce-2", "POST", base + "com.atproto.test.upload", 1 }
            << TestCase { 0, "resource-nonce-3", "GET", base + "com.atproto.test.get", 1 };

    for (const auto &item : cases) {
        m_server.m_resourceNonce = item.server_nonce;
        m_server.m_rotateAlways = false;
        m_server.m_resourceChallengeCount = 0;
        m_resourceAuthorizations.clear();
        m_resourceDPops.clear();

        TestAccess access;
        access.setAccount(account);
        QSignalSpy spy(&access, SIGNAL(finished(bool)));
        if (item.kind == 0) {
            access.testGet();
        } else if (item.kind == 1) {
            access.testPost();
        } else {
            access.testPostWithImage(image.fileName());
        }
        spy.wait();
        QCOMPARE(spy.count(), 1);
        QVERIFY(spy.takeFirst().at(0).toBool());
        QCOMPARE(m_server.m_resourceChallengeCount, item.challenge_count);
        QCOMPARE(store->nonce(uuid, QUrl(base)), item.server_nonce);

        QCOMPARE(m_resourceAuthorizations.length(), 1);
        QCOMPARE(m_resourceAuthorizations.first(), QByteArray("DPoP ") + access_token.toUtf8());
        const QByteArrayList parts = m_resourceDPops.first().split('.');
        QCOMPARE(parts.length(), 3);
        const QJsonObject jwk = decode_jwt_part(parts.at(0)).value("jwk").toObject();
        QCOMPARE(jwk.value("x").toString(), QString::fromUtf8(x_coord));
        QCOMPARE(jwk.value("y").toString(), QString::fromUtf8(y_coord));
        const QJsonObject payload = decode_jwt_part(parts.at(1));
        QCOMPARE(payload.value("htm").toString(), item.htm);
        // htuはクエリを含まない
        QCOMPARE(payload.value("htu").toString(), item.htu);
        QCOMPARE(payload.value("ath").toString(), ath);
        QCOMPARE(payload.value("nonce").toString(), item.server_nonce);
    }

    {
        // 再送してもnonceを要求される場合は1回で諦める
        m_server.m_resourceNonce = "resource-nonce-4";
        m_server.m_rotateAlways = true;
        m_server.m_resourceChallengeCount = 0;
        TestAccess access;
        access.setAccount(account);
        QSignalSpy spy(&access, SIGNAL(finished(bool)));
        access.testGet();
        spy.wait();
        QCOMPARE(spy.count(), 1);
        QVERIFY(!spy.takeFirst().at(0).toBool());
        QCOMPARE(m_server.m_resourceChallengeCount, 2);
        QCOMPARE(access.errorCode(), QString("use_dpop_nonce"));
        m_server.m_rotateAlways = false;
    }

    {
        // DPoPの鍵がないOAuthのアカウントはリクエストを送らない
        m_resourceAuthorizations.clear();
        AtProtocolInterface::AccountData no_key = account;
        no_key.uuid = "uuid-no-key";
        for (int i = 0; i < 3; i++) {
            TestAccess access;
            access.setAccount(no_key);
            QSignalSpy spy(&access, SIGNAL(finished(bool)));
            if (i == 0) {
                access.testGet();
            } else if (i == 1) {
                access.testPost();
            } else {
                access.testPostWithImage(image.fileName());
            }
            QCOMPARE(spy.count(), 1);
            QVERIFY(!spy.takeFirst().at(0).toBool());
            QCOMPARE(access.errorCode(), QString("IncompleteAuthenticationInformation"));
        }
        QCOMPARE(m_resourceAuthorizations.length(), 0);
    }

    store->removeSession(uuid);
    QVERIFY(!store->hasSession(uuid));
    QVERIFY(store->nonce(uuid, QUrl(base)).isEmpty());
    m_server.m_resourceNonce.clear();
}

void oauth_test::test_account_manager_oauth()
{
    AccountManager *manager = AccountManager::getInstance();
    DPopSessionStore *store = DPopSessionStore::getInstance();
    manager->clear();
    QFile::remove(Common::appDataFolder() + "/account.json");
    m_server.m_resourceNonce = "resource-nonce-account";
    m_server.m_rotateAlways = false;
    m_server.m_resourceChallengeCount = 0;
    m_dPopPayloads.clear();
    m_resourceAuthorizations.clear();

    const QString did = "did:plc:ipj5qejfoqu6eukvt72uhyit";
    const QString pds = QString("http://127.0.0.1:%1/response/4").arg(m_listenPort);
    const QString token_endpoint =
            QString("http://127.0.0.1:%1/response/2/oauth/token").arg(m_listenPort);
    const QByteArray private_key = generate_private_key_pem();

    OAuthSession session;
    session.handle = "ioriayane.test";
    session.service_endpoint = pds;
    session.issuer = QString("http://127.0.0.1:%1").arg(m_listenPort);
    session.token_endpoint = token_endpoint;
    session.revocation_endpoint =
            QString("http://127.0.0.1:%1/response/2/oauth/revoke").arg(m_listenPort);
    session.dpop_private_key = private_key;
    session.token.access_token = "first access token";
    session.token.refresh_token = "first refresh token";
    session.token.token_type = "DPoP";
    session.token.sub = did;
    session.token.scope =
            "atproto include:app.bsky.authFullApp?aud=did:web:api.bsky.app%23bsky_appview";
    session.token.expires_in = 2677;

    // 追加
    const QString uuid = manager->updateOAuthAccount(QString(), "https://bsky.social", session);
    QVERIFY(!uuid.isEmpty());
    QCOMPARE(manager->count(), 1);
    {
        const AtProtocolInterface::AccountData account = manager->getAccount(uuid);
        QCOMPARE(account.auth_type, AtProtocolInterface::AuthType::OAuth);
        QCOMPARE(account.service, QString("https://bsky.social"));
        QCOMPARE(account.service_endpoint, pds);
        QCOMPARE(account.did, did);
        QCOMPARE(account.handle, QString("ioriayane.test"));
        QCOMPARE(account.accessJwt, QString("first access token"));
        QCOMPARE(account.status, AtProtocolInterface::AccountStatus::Authorized);
        QVERIFY(account.password.isEmpty());
        // chatの権限がないのでDMは使えない
        QVERIFY(!account.scope.contains(AtProtocolInterface::AccountScope::DirectMessage));
    }
    QVERIFY(store->hasSession(uuid));

    // 保存したファイルに秘密情報が平文で含まれないこと
    {
        QFile file(Common::appDataFolder() + "/account.json");
        QVERIFY(file.open(QFile::ReadOnly));
        const QByteArray raw = file.readAll();
        QVERIFY(raw.contains("\"auth_type\": \"oauth\""));
        QVERIFY(!raw.contains("first refresh token"));
        QVERIFY(!raw.contains("first access token"));
        QVERIFY(!raw.contains("PRIVATE KEY"));
    }

    // 同じDIDなら同じuuidのまま置き換える
    session.token.scope =
            "atproto include:chat.bsky.authFullChatClient?aud=did:web:api.bsky.chat%23bsky_chat";
    QCOMPARE(manager->updateOAuthAccount(QString(), "https://bsky.social", session), uuid);
    QCOMPARE(manager->count(), 1);
    QVERIFY(manager->getAccount(uuid).scope.contains(
            AtProtocolInterface::AccountScope::DirectMessage));

    // refresh(tokenの取得、プロフィールの取得はDPoPで行う)
    {
        QSignalSpy spy(manager, SIGNAL(updatedAccount(const QString &)));
        QSignalSpy spy_error(manager, SIGNAL(errorOccurred(const QString &, const QString &)));
        manager->refreshSession(manager->indexAt(uuid));
        // 同時にrefreshしてもtokenのリクエストは1回
        manager->refreshSession(manager->indexAt(uuid));
        spy.wait(30 * 1000);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.takeFirst().at(0).toString(), uuid);
        for (const auto &error : spy_error) {
            qDebug().noquote() << "error :" << error.at(0).toString() << error.at(1).toString();
        }
        QCOMPARE(spy_error.count(), 0);
    }
    QCOMPARE(m_dPopPayloads.length(), 1);
    QCOMPARE(m_dPopPayloads.first().value("htu").toString(), token_endpoint);
    // PDSへのリクエスト(getPreferences/getProfile)はrefreshしたtokenとDPoPで送る
    QVERIFY(m_resourceAuthorizations.contains(QByteArray("DPoP access token")));
    for (const auto &authorization : std::as_const(m_resourceAuthorizations)) {
        QVERIFY(!authorization.startsWith("Bearer"));
    }
    // PDSのnonceは最初の1回だけ要求される
    QCOMPARE(m_server.m_resourceChallengeCount, 1);
    {
        const AtProtocolInterface::AccountData account = manager->getAccount(uuid);
        QCOMPARE(account.accessJwt, QString("access token"));
        QCOMPARE(account.refreshJwt, QString("refresh token"));
        QCOMPARE(account.displayName, QString("Iori Ayane"));
        QCOMPARE(account.service_endpoint, pds);
        QCOMPARE(account.status, AtProtocolInterface::AccountStatus::Authorized);
        // tokenのレスポンスのscope(transition:chat.bsky)でDMを判断する
        QVERIFY(account.scope.contains(AtProtocolInterface::AccountScope::DirectMessage));
    }

    // 再起動した想定で読み込み直す
    manager->clear();
    store->clear();
    QCOMPARE(manager->count(), 0);
    {
        QSignalSpy spy(manager, SIGNAL(updatedAccount(const QString &)));
        manager->load();
        spy.wait(30 * 1000);
        QCOMPARE(spy.count(), 1);
    }
    QCOMPARE(manager->count(), 1);
    QVERIFY(store->hasSession(uuid));
    {
        const AtProtocolInterface::AccountData account = manager->getAccount(uuid);
        QCOMPARE(account.auth_type, AtProtocolInterface::AuthType::OAuth);
        QCOMPARE(account.did, did);
        QCOMPARE(account.handle, QString("ioriayane.test"));
        QCOMPARE(account.service_endpoint, pds);
        QCOMPARE(account.status, AtProtocolInterface::AccountStatus::Authorized);
        QVERIFY(account.password.isEmpty());
    }
    QCOMPARE(m_dPopPayloads.length(), 2);
    // 保存した鍵で署名していること
    {
        Es256 key;
        QVERIFY(key.loadPrivateKeyPem(private_key));
        QByteArray x_coord;
        QByteArray y_coord;
        QVERIFY(key.getAffineCoordinates(x_coord, y_coord));
        QCOMPARE(m_dPopHeaders.last().value("jwk").toObject().value("x").toString(),
                 QString::fromUtf8(x_coord));
    }

    // OAuthのアカウントはパスワードでセッションを作り直さない
    {
        QSignalSpy spy_error(manager, SIGNAL(errorOccurred(const QString &, const QString &)));
        manager->createSession(manager->indexAt(uuid));
        QCOMPARE(spy_error.count(), 1);
        QCOMPARE(spy_error.takeFirst().at(0).toString(), QString("OAuthLoginRequired"));
        QCOMPARE(manager->getAccount(uuid).status,
                 AtProtocolInterface::AccountStatus::Unauthorized);
    }

    // refreshに失敗したら再ログインが必要な状態になる
    {
        session.token_endpoint =
                QString("http://127.0.0.1:%1/response/2/oauth/token_notfound").arg(m_listenPort);
        manager->updateOAuthAccount(uuid, "https://bsky.social", session);
        QSignalSpy spy_error(manager, SIGNAL(errorOccurred(const QString &, const QString &)));
        manager->refreshSession(manager->indexAt(uuid));
        spy_error.wait(10 * 1000);
        QCOMPARE(spy_error.count(), 1);
        QCOMPARE(manager->getAccount(uuid).status,
                 AtProtocolInterface::AccountStatus::Unauthorized);
        // 失敗してもセッションの情報は残す
        QCOMPARE(manager->getAccount(uuid).auth_type, AtProtocolInterface::AuthType::OAuth);
        QVERIFY(store->hasSession(uuid));
    }

    // パスワード方式に切り替えるとOAuthのセッションは失効させる
    m_revokeBodies.clear();
    manager->updateAccount(uuid, "https://bsky.social", "ioriayane.test", "password", did,
                           "ioriayane.test", "email", "access_jwt", "refresh_jwt", true);
    for (int i = 0; i < 50 && m_revokeBodies.isEmpty(); i++) {
        QTest::qWait(100);
    }
    QCOMPARE(m_revokeBodies.length(), 1);
    QCOMPARE(QUrlQuery(QString::fromUtf8(m_revokeBodies.first()))
                     .queryItemValue("token", QUrl::FullyDecoded),
             QString("first refresh token"));
    QCOMPARE(manager->getAccount(uuid).auth_type, AtProtocolInterface::AuthType::Password);
    QVERIFY(!store->hasSession(uuid));
    {
        QFile file(Common::appDataFolder() + "/account.json");
        QVERIFY(file.open(QFile::ReadOnly));
        const QByteArray raw = file.readAll();
        QVERIFY(raw.contains("\"auth_type\": \"password\""));
        QVERIFY(!raw.contains("\"oauth\""));
    }

    // 削除するとDPoPのセッションも消え、サーバーでも失効させる
    session.token.refresh_token = "last refresh token";
    manager->updateOAuthAccount(uuid, "https://bsky.social", session);
    QVERIFY(store->hasSession(uuid));
    m_revokeBodies.clear();
    manager->removeAccount(uuid);
    QCOMPARE(manager->count(), 0);
    QVERIFY(!store->hasSession(uuid));
    for (int i = 0; i < 50 && m_revokeBodies.isEmpty(); i++) {
        QTest::qWait(100);
    }
    QCOMPARE(m_revokeBodies.length(), 1);
    QCOMPARE(QUrlQuery(QString::fromUtf8(m_revokeBodies.first()))
                     .queryItemValue("token", QUrl::FullyDecoded),
             QString("last refresh token"));

    m_server.m_resourceNonce.clear();
    manager->clear();
    QFile::remove(Common::appDataFolder() + "/account.json");
}

void oauth_test::test_account_manager_legacy_encryption()
{
    // 以前のバージョン(IV固定の暗号化)で保存したaccount.jsonを読み込み、保存し直すと新しい形式になる
    AccountManager *manager = AccountManager::getInstance();
    manager->clear();
    DPopSessionStore::getInstance()->clear();
    const QString path = Common::appDataFolder() + "/account.json";
    QFile::remove(path);

    const QString uuid = "uuid-legacy-encryption";
    const QByteArray private_key = generate_private_key_pem();
    Encryption encryption;
    {
        // パスワード方式とOAuthのアカウント(ログインはできないサービスにしておく)
        const QString service = QString("http://127.0.0.1:%1/response/notfound").arg(m_listenPort);
        QJsonObject password_account;
        password_account["uuid"] = uuid;
        password_account["is_main"] = true;
        password_account["service"] = service;
        password_account["identifier"] = "legacy.test";
        password_account["password"] = encryption.encryptLegacy("legacy password");
        password_account["refresh_jwt"] = encryption.encryptLegacy("legacy refresh jwt");

        QJsonObject oauth;
        oauth["issuer"] = service;
        oauth["token_endpoint"] = service + "/oauth/token";
        oauth["dpop_private_key"] = encryption.encryptLegacy(QString::fromUtf8(private_key));
        QJsonObject oauth_account;
        oauth_account["uuid"] = uuid + "-oauth";
        oauth_account["is_main"] = false;
        oauth_account["service"] = service;
        oauth_account["identifier"] = "legacy-oauth.test";
        oauth_account["password"] = encryption.encryptLegacy(QString());
        oauth_account["refresh_jwt"] = encryption.encryptLegacy("legacy oauth refresh token");
        oauth_account["auth_type"] = "oauth";
        oauth_account["did"] = "did:plc:legacyoauth";
        oauth_account["handle"] = "legacy-oauth.test";
        oauth_account["service_endpoint"] = service;
        oauth_account["oauth"] = oauth;

        QJsonArray accounts;
        accounts.append(password_account);
        accounts.append(oauth_account);
        Common::saveJsonDocument(QJsonDocument(accounts), "account.json");
    }

    {
        // 読み込むとセッションの復元を試みて失敗するまで待つ
        QSignalSpy spy(manager, SIGNAL(finished()));
        manager->load();
        spy.wait(10 * 1000);
    }
    QCOMPARE(manager->count(), 2);
    QCOMPARE(manager->getAccount(uuid).password, QString("legacy password"));
    // OAuthの鍵も読めている
    QVERIFY(DPopSessionStore::getInstance()->hasSession(uuid + "-oauth"));

    manager->save();
    {
        QFile file(path);
        QVERIFY(file.open(QFile::ReadOnly));
        const QJsonArray accounts = QJsonDocument::fromJson(file.readAll()).array();
        QCOMPARE(accounts.count(), 2);
        const QJsonObject password_account = accounts.at(0).toObject();
        QVERIFY(password_account.value("password").toString().startsWith("v2:"));
        QVERIFY(password_account.value("refresh_jwt").toString().startsWith("v2:"));
        QCOMPARE(encryption.decrypt(password_account.value("password").toString()),
                 QString("legacy password"));
        const QJsonObject oauth_account = accounts.at(1).toObject();
        const QString key =
                oauth_account.value("oauth").toObject().value("dpop_private_key").toString();
        QVERIFY(key.startsWith("v2:"));
        QCOMPARE(encryption.decrypt(key).toUtf8(), private_key);
    }

    manager->clear();
    DPopSessionStore::getInstance()->clear();
    QFile::remove(path);
}

void oauth_test::test_oauth_revoke()
{
    const QString revoke_endpoint =
            QString("http://127.0.0.1:%1/response/2/oauth/revoke").arg(m_listenPort);
    m_server.m_nonce = "nonce-revoke";
    m_server.m_rotateAlways = false;
    m_server.m_challengeCount = 0;

    AtProtocolType::OauthDefs::TokenResponse token;
    token.access_token = "access token";
    token.refresh_token = "refresh token";
    token.sub = "did:plc:ipj5qejfoqu6eukvt72uhyit";

    {
        // エンドポイントを指定して失効
        m_revokeBodies.clear();
        m_dPopPayloads.clear();
        Authorization oauth;
        QVERIFY(oauth.setDPopPrivateKey(generate_private_key_pem()));
        oauth.setToken(token);
        oauth.setRevocationEndpoint(revoke_endpoint);
        QSignalSpy spy(&oauth, SIGNAL(revokeFinished(bool)));
        oauth.revokeToken();
        spy.wait(30 * 1000);
        QCOMPARE(spy.count(), 1);
        QVERIFY(spy.takeFirst().at(0).toBool());
        // nonceを要求されて1回再送
        QCOMPARE(m_server.m_challengeCount, 1);
        QCOMPARE(m_revokeBodies.length(), 1);
        const QUrlQuery query(QString::fromUtf8(m_revokeBodies.first()));
        QCOMPARE(query.queryItemValue("token", QUrl::FullyDecoded), QString("refresh token"));
        QCOMPARE(query.queryItemValue("token_type_hint", QUrl::FullyDecoded),
                 QString("refresh_token"));
        QCOMPARE(query.queryItemValue("client_id", QUrl::FullyDecoded),
                 QString("https://oauth.hagoromo.relog.tech/oauth-client-metadata.json"));
        QCOMPARE(m_dPopPayloads.last().value("htu").toString(), revoke_endpoint);
        QCOMPARE(m_dPopPayloads.last().value("nonce").toString(), QString("nonce-revoke"));
    }
    {
        // エンドポイントを知らないときは認可サーバーのメタデータから取得する
        m_revokeBodies.clear();
        Authorization oauth;
        QVERIFY(oauth.setDPopPrivateKey(generate_private_key_pem()));
        oauth.setToken(token);
        oauth.setAuthorizationServer(QString("http://127.0.0.1:%1/response/2").arg(m_listenPort));
        QSignalSpy spy(&oauth, SIGNAL(revokeFinished(bool)));
        oauth.revokeToken();
        spy.wait(30 * 1000);
        QCOMPARE(spy.count(), 1);
        QVERIFY(spy.takeFirst().at(0).toBool());
        QCOMPARE(oauth.revocationEndpoint(), revoke_endpoint);
        QCOMPARE(m_revokeBodies.length(), 1);
    }
    {
        // 鍵やtokenがなければ送らない
        m_revokeBodies.clear();
        Authorization no_key;
        no_key.setToken(token);
        no_key.setRevocationEndpoint(revoke_endpoint);
        QSignalSpy spy(&no_key, SIGNAL(revokeFinished(bool)));
        no_key.revokeToken();
        QCOMPARE(spy.count(), 1);
        QVERIFY(!spy.takeFirst().at(0).toBool());

        Authorization no_token;
        QVERIFY(no_token.setDPopPrivateKey(generate_private_key_pem()));
        no_token.setRevocationEndpoint(revoke_endpoint);
        QSignalSpy spy2(&no_token, SIGNAL(revokeFinished(bool)));
        no_token.revokeToken();
        QCOMPARE(spy2.count(), 1);
        QVERIFY(!spy2.takeFirst().at(0).toBool());

        // エンドポイントもメタデータの取得先もない
        Authorization no_endpoint;
        QVERIFY(no_endpoint.setDPopPrivateKey(generate_private_key_pem()));
        no_endpoint.setToken(token);
        QSignalSpy spy3(&no_endpoint, SIGNAL(revokeFinished(bool)));
        no_endpoint.revokeToken();
        QCOMPARE(spy3.count(), 1);
        QVERIFY(!spy3.takeFirst().at(0).toBool());
        QCOMPARE(m_revokeBodies.length(), 0);
    }
    m_server.m_nonce.clear();
}

// HttpAccessのワーカースレッドからもログが出るので排他する
static QMutex g_capturedLogsMutex;
static QStringList g_capturedLogs;
static QtMessageHandler g_previousHandler = nullptr;
static void captureLogHandler(QtMsgType type, const QMessageLogContext &context,
                              const QString &message)
{
    QMutexLocker locker(&g_capturedLogsMutex);
    g_capturedLogs.append(message);
    if (g_previousHandler != nullptr) {
        g_previousHandler(type, context, message);
    }
}

void oauth_test::test_no_secret_in_logs()
{
    // 各処理のログにトークン、パスワード、DPoPのproofが出ないこと
    const QString access_token = "secret-access-token-for-log-test";
    const QString refresh_token = "secret-refresh-token-for-log-test";
    const QString uuid = "uuid-log-test";
    QVERIFY(DPopSessionStore::getInstance()->setPrivateKey(uuid, generate_private_key_pem()));

    {
        QMutexLocker locker(&g_capturedLogsMutex);
        g_capturedLogs.clear();
        g_previousHandler = qInstallMessageHandler(captureLogHandler);
    }

    for (int i = 0; i < 2; i++) {
        // Bearer(パスワード方式)とDPoP(OAuth)
        AtProtocolInterface::AccountData account;
        account.uuid = uuid;
        account.auth_type = (i == 0) ? AtProtocolInterface::AuthType::Password
                                     : AtProtocolInterface::AuthType::OAuth;
        account.service = QString("http://127.0.0.1:%1/response/4").arg(m_listenPort);
        account.accessJwt = access_token;
        account.refreshJwt = refresh_token;
        TestAccess access;
        access.setAccount(account);
        QSignalSpy spy(&access, SIGNAL(finished(bool)));
        access.testGet();
        spy.wait();
        QCOMPARE(spy.count(), 1);
    }
    {
        // OAuthのrefreshと失効
        AtProtocolType::OauthDefs::TokenResponse token;
        token.refresh_token = refresh_token;
        token.sub = "did:plc:ipj5qejfoqu6eukvt72uhyit";
        Authorization oauth;
        QVERIFY(oauth.setDPopPrivateKey(generate_private_key_pem()));
        oauth.setToken(token);
        oauth.setTokenEndopoint(
                QString("http://127.0.0.1:%1/response/2/oauth/token").arg(m_listenPort));
        oauth.setRevocationEndpoint(
                QString("http://127.0.0.1:%1/response/2/oauth/revoke").arg(m_listenPort));
        oauth.makeClientId();
        QSignalSpy spy(&oauth, SIGNAL(finished(bool)));
        oauth.requestToken(true);
        spy.wait();
        QCOMPARE(spy.count(), 1);
        // 応答のtoken("access token"/"refresh token")も出さない
        oauth.setToken(token);
        QSignalSpy spy_revoke(&oauth, SIGNAL(revokeFinished(bool)));
        oauth.revokeToken();
        spy_revoke.wait();
        QCOMPARE(spy_revoke.count(), 1);
    }

    QStringList captured_logs;
    {
        QMutexLocker locker(&g_capturedLogsMutex);
        qInstallMessageHandler(g_previousHandler);
        g_previousHandler = nullptr;
        captured_logs = g_capturedLogs;
    }
    DPopSessionStore::getInstance()->removeSession(uuid);

    QVERIFY(!captured_logs.isEmpty());
    for (const auto &log : std::as_const(captured_logs)) {
        QVERIFY2(!log.contains(access_token), qPrintable(log));
        QVERIFY2(!log.contains(refresh_token), qPrintable(log));
        QVERIFY2(!log.contains(QStringLiteral("\"access token\"")), qPrintable(log));
        QVERIFY2(!log.contains(QStringLiteral("\"refresh token\"")), qPrintable(log));
        // DPoPのproof(JWTのヘッダー部分 {"alg":"ES256" をbase64urlにしたもの)
        QVERIFY2(!log.contains(QStringLiteral("eyJhbGciOiJFUzI1NiI")), qPrintable(log));
    }
}

void oauth_test::test_jwt()
{
    Es256 key;
    QVERIFY(key.generateKey());

    const QString access_token = "access token";
    QByteArray jwt =
            JsonWebToken::generate(key, "https://hoge/path?query=1#fragment", "GET",
                                   "O_m5dyvKO7jNfnsfuYwB5GflhTuVaqCub4x3xVKqJ9Y", access_token);
    qDebug().noquote() << jwt;
    verify_jwt(jwt, nullptr);
    verify_jwt(jwt, key.pKey());

    const QByteArrayList parts = jwt.split('.');
    QCOMPARE(parts.length(), 3);
    const QJsonObject header = decode_jwt_part(parts.at(0));
    QCOMPARE(header.value("alg").toString(), QString("ES256"));
    QCOMPARE(header.value("typ").toString(), QString("dpop+jwt"));
    QVERIFY(!header.value("jwk").toObject().contains("d"));

    const QJsonObject payload = decode_jwt_part(parts.at(1));
    QCOMPARE(payload.value("htm").toString(), QString("GET"));
    QCOMPARE(payload.value("htu").toString(), QString("https://hoge/path"));
    QCOMPARE(payload.value("nonce").toString(),
             QString("O_m5dyvKO7jNfnsfuYwB5GflhTuVaqCub4x3xVKqJ9Y"));
    QCOMPARE(payload.value("ath").toString(),
             QString::fromUtf8(
                     QCryptographicHash::hash(access_token.toUtf8(), QCryptographicHash::Sha256)
                             .toBase64(QByteArray::Base64UrlEncoding
                                       | QByteArray::OmitTrailingEquals)));
    QVERIFY(qAbs(payload.value("iat").toInteger() - QDateTime::currentSecsSinceEpoch()) <= 5);
    QVERIFY(!payload.value("jti").toString().isEmpty());
    // RFC 9449で定義されていないクレームは付けない
    QVERIFY(!payload.contains("exp"));
    QVERIFY(!payload.contains("iss"));
    QVERIFY(!payload.contains("sub"));

    // nonceとathは指定したときだけ付く
    const QByteArray jwt2 = JsonWebToken::generate(key, "https://hoge/path", "POST", QString());
    const QJsonObject payload2 = decode_jwt_part(jwt2.split('.').at(1));
    QVERIFY(!payload2.contains("nonce"));
    QVERIFY(!payload2.contains("ath"));
    QVERIFY(payload.value("jti") != payload2.value("jti"));

    // 鍵がない場合は生成しない
    Es256 empty_key;
    QVERIFY(JsonWebToken::generate(empty_key, "https://hoge", "GET", QString()).isEmpty());
}

void oauth_test::test_es256()
{
    Es256 key;
    QVERIFY(!key.isValid());
    QVERIFY(key.sign("header.payload").isEmpty());
    QVERIFY(key.privateKeyPem().isEmpty());

    QVERIFY(key.generateKey());
    QVERIFY(key.isValid());
    {
        QString message = "header.payload";
        QByteArray sign = key.sign(message.toUtf8());
        QCOMPARE(sign.length(), 64);
        QByteArray jwt = message.toUtf8() + '.'
                + sign.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
        verify_jwt(jwt, key.pKey());
    }

    // PEMで保存・復元できること
    const QByteArray pem = key.privateKeyPem();
    QVERIFY(pem.startsWith("-----BEGIN PRIVATE KEY-----"));
    Es256 restored;
    QVERIFY(restored.loadPrivateKeyPem(pem));
    {
        QByteArray x1, y1, x2, y2;
        QVERIFY(key.getAffineCoordinates(x1, y1));
        QVERIFY(restored.getAffineCoordinates(x2, y2));
        QCOMPARE(x1, x2);
        QCOMPARE(y1, y2);

        QString message = "header2.payload2";
        QByteArray sign = restored.sign(message.toUtf8());
        QByteArray jwt = message.toUtf8() + '.'
                + sign.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
        verify_jwt(jwt, key.pKey());
    }

    // 鍵は生成するたびに異なること
    {
        Es256 other;
        QVERIFY(other.generateKey());
        QVERIFY(other.privateKeyPem() != pem);
    }

    // JWKの座標は先頭が0でも32バイト固定
    for (int i = 0; i < 300; i++) {
        Es256 temp;
        QVERIFY(temp.generateKey());
        QByteArray x_coord, y_coord;
        QVERIFY(temp.getAffineCoordinates(x_coord, y_coord));
        QCOMPARE(QByteArray::fromBase64(x_coord, QByteArray::Base64UrlEncoding).length(), 32);
        QCOMPARE(QByteArray::fromBase64(y_coord, QByteArray::Base64UrlEncoding).length(), 32);
        QCOMPARE(temp.sign("header.payload").length(), 64);
    }

    // 不正な鍵は読み込まない
    QVERIFY(!restored.loadPrivateKeyPem(QByteArray()));
    QVERIFY(!restored.isValid());
    QVERIFY(!restored.loadPrivateKeyPem("-----BEGIN PRIVATE KEY-----\nhoge\n"
                                        "-----END PRIVATE KEY-----\n"));
    {
        // P-256以外
        EVP_PKEY *pkey = nullptr;
        EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr);
        QVERIFY(pctx);
        QVERIFY(EVP_PKEY_keygen_init(pctx) > 0);
        QVERIFY(EVP_PKEY_CTX_set_ec_paramgen_curve_nid(pctx, NID_secp384r1) > 0);
        QVERIFY(EVP_PKEY_keygen(pctx, &pkey) > 0);
        EVP_PKEY_CTX_free(pctx);
        BIO *bio = BIO_new(BIO_s_mem());
        QVERIFY(PEM_write_bio_PrivateKey(bio, pkey, nullptr, nullptr, 0, nullptr, nullptr) > 0);
        char *data = nullptr;
        long length = BIO_get_mem_data(bio, &data);
        const QByteArray p384_pem(data, static_cast<int>(length));
        BIO_free(bio);
        EVP_PKEY_free(pkey);

        QVERIFY(!restored.loadPrivateKeyPem(p384_pem));
        QVERIFY(!restored.isValid());
    }
}

#endif // AUTH_TEST_IN_PRODUCTION_ENVIRONMENT

QJsonObject oauth_test::decode_jwt_part(const QByteArray &part)
{
    return QJsonDocument::fromJson(QByteArray::fromBase64(part,
                                                          QByteArray::Base64UrlEncoding
                                                                  | QByteArray::OmitTrailingEquals))
            .object();
}

QByteArray oauth_test::generate_private_key_pem()
{
    Es256 key;
    key.generateKey();
    return key.privateKeyPem();
}

void oauth_test::test_get(const QString &url, const QByteArray &except_data)
{
    qDebug() << "test_get url" << url;
    QNetworkRequest request((QUrl(url)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, [=](QNetworkReply *reply) {
        qDebug() << "test_get reply" << reply->error() << reply->url();

        QJsonDocument json_doc = QJsonDocument::fromJson(reply->readAll());

        QCOMPARE(reply->error(), QNetworkReply::NoError);
        QCOMPARE(reply->readAll(), except_data);

        reply->deleteLater();
        manager->deleteLater();
    });
    manager->get(request);
}

void oauth_test::verify_jwt(const QByteArray &jwt, EVP_PKEY *pkey)
{
    EC_KEY *ec_key = nullptr;

    const QByteArrayList jwt_parts = jwt.split('.');
    QCOMPARE(jwt_parts.length(), 3);

    QByteArray message = jwt_parts[0] + '.' + jwt_parts[1];
    QByteArray sig = QByteArray::fromBase64(
            jwt_parts.last(), QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
    QCOMPARE(sig.length(), 64);
    QByteArray header = QByteArray::fromBase64(
            jwt_parts.first(), QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);

    bool use_jwk = false;
    QJsonDocument json_doc = QJsonDocument::fromJson(header);
    if (json_doc.object().contains("jwk")) {
        QJsonObject jwk_obj = json_doc.object().value("jwk").toObject();
        QByteArray x_coord = QByteArray::fromBase64(jwk_obj.value("x").toString().toUtf8(),
                                                    QByteArray::Base64UrlEncoding
                                                            | QByteArray::OmitTrailingEquals);
        QByteArray y_coord = QByteArray::fromBase64(jwk_obj.value("y").toString().toUtf8(),
                                                    QByteArray::Base64UrlEncoding
                                                            | QByteArray::OmitTrailingEquals);
        QVERIFY(!x_coord.isEmpty());
        QVERIFY(!y_coord.isEmpty());

        BIGNUM *x = BN_bin2bn(reinterpret_cast<const unsigned char *>(x_coord.constData()),
                              x_coord.length(), nullptr);
        BIGNUM *y = BN_bin2bn(reinterpret_cast<const unsigned char *>(y_coord.constData()),
                              y_coord.length(), nullptr);
        QVERIFY(x);
        QVERIFY(y);

        ec_key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
        QVERIFY(ec_key);
        QVERIFY(EC_KEY_set_public_key_affine_coordinates(ec_key, x, y));

        pkey = EVP_PKEY_new();
        QVERIFY(EVP_PKEY_assign_EC_KEY(pkey, ec_key));

        BN_free(y);
        BN_free(x);

        use_jwk = true;
    }

    // convert IEEE P1363 to DER
    QByteArray sig_rr = sig.left(32);
    QByteArray sig_ss = sig.right(32);
    BIGNUM *ec_sig_r = NULL;
    BIGNUM *ec_sig_s = NULL;
    ec_sig_r = BN_bin2bn(reinterpret_cast<const unsigned char *>(sig_rr.constData()),
                         sig_rr.length(), NULL);
    ec_sig_s = BN_bin2bn(reinterpret_cast<const unsigned char *>(sig_ss.constData()),
                         sig_ss.length(), NULL);
    QVERIFY(ec_sig_r != NULL);
    QVERIFY(ec_sig_s != NULL);

    ECDSA_SIG *ec_sig = ECDSA_SIG_new();
    QCOMPARE(ECDSA_SIG_set0(ec_sig, ec_sig_r, ec_sig_s), 1);

    unsigned char *der_sig = NULL;
    int der_sig_len = i2d_ECDSA_SIG(ec_sig, &der_sig);
    QVERIFY(der_sig_len > 0);

    // ECDSA署名の検証
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    QCOMPARE_NE(pkey, nullptr);
    QVERIFY(EVP_DigestVerifyInit(mdctx, nullptr, EVP_sha256(), nullptr, pkey) > 0);
    QVERIFY(EVP_DigestVerifyUpdate(mdctx, message.constData(), message.length()) > 0);
    QVERIFY(EVP_DigestVerifyFinal(mdctx, der_sig, der_sig_len) > 0);
    EVP_MD_CTX_free(mdctx);

    OPENSSL_free(der_sig);
    ECDSA_SIG_free(ec_sig);
    // BN_free(ec_sig_s); // ECDSA_SIG_freeで解放される
    // BN_free(ec_sig_r); // ECDSA_SIG_freeで解放される

    if (use_jwk) {
        EVP_PKEY_free(pkey);
        // EC_KEY_free(ec_key); EVP_PKEY_freeで解放される
    }
}

QTEST_MAIN(oauth_test)

#include "tst_oauth_test.moc"
