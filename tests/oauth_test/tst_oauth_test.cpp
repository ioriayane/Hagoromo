#include <QtTest>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDesktopServices>

#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/bn.h>

#include "tools/authorization.h"
#include "tools/jsonwebtoken.h"
#include "tools/es256.h"
#include "tools/identityresolver.h"
#include "extension/well-known/wellknownatprotodid.h"
#include "http/simplehttpserver.h"

#include <QHttpHeaders>

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
        if (!m_nonce.isEmpty() && (path.endsWith("/oauth/par") || path.endsWith("/oauth/token"))
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

class oauth_test : public QObject
{
    Q_OBJECT

public:
    oauth_test();
    ~oauth_test();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_oauth_process();
    void test_oauth_server();
    void test_oauth();
    void test_oauth_dpop_nonce_retry_limit();
    void test_oauth_token_validation();
    void test_oauth_token_request_lock();
    void test_identity_resolver();
    void test_client_metadata();
    void test_identity_resolver_online();
    void test_well_known_atproto_did();
    void test_jwt();
    void test_es256();

private:
    DPopNonceServer m_server;
    quint16 m_listenPort;

    // サーバーが受け付けたPAR/tokenリクエストのDPoP
    QList<QJsonObject> m_dPopHeaders;
    QList<QJsonObject> m_dPopPayloads;
    // サーバーが受け付けたPARのリクエストボディ
    QList<QByteArray> m_parBodies;

    void test_get(const QString &url, const QByteArray &except_data);
    void verify_jwt(const QByteArray &jwt, EVP_PKEY *pkey);
    static QJsonObject decode_jwt_part(const QByteArray &part);
    static QByteArray generate_private_key_pem();
};

oauth_test::oauth_test()
{
    QCoreApplication::setOrganizationName(QStringLiteral("relog"));
    QCoreApplication::setApplicationName(QStringLiteral("Hagoromo_unittest"));

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
                if (path.endsWith("/oauth/par") || path.endsWith("/oauth/token")) {
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

void oauth_test::test_oauth_server()
{
    Authorization oauth;
#if 0
    {
        QSignalSpy spy(&oauth, SIGNAL(madeRequestUrl(const QString &)));
        oauth.start("https://bsky.social", "ioriayane.bsky.social");
        spy.wait();
        QCOMPARE(spy.count(), 1);
        QList<QVariant> arguments = spy.takeFirst();
        QString request_url = arguments.at(0).toString();
        qDebug().noquote() << "request url:" << request_url;
        QDesktopServices::openUrl(request_url);
    }
    {
        QSignalSpy spy(&oauth, SIGNAL(finished(bool)));
        spy.wait(5 * 60 * 1000);
        QCOMPARE(spy.count(), 1);
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }
    qDebug().noquote() << "DPoP for test";
    qDebug().noquote() << "DPoP private key size" << oauth.dPopPrivateKey().size();
#elif 0
    AtProtocolType::OauthDefs::TokenResponse token;
    token.refresh_token = "ref-121f89618c436";
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

void oauth_test::test_oauth()
{
    // response/1 : pds
    // response/2 : entry-way

    Authorization oauth;
    oauth.setRedirectTimeout(20);
    oauth.setPlcDirectory(QString("http://localhost:%1/response/plc").arg(m_listenPort));

    // DNS/HTTPSでは解決できないハンドルにして、pdsのresolveHandleで解決させる
    QString pds = QString("http://localhost:%1/response/2").arg(m_listenPort);
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
    QCOMPARE(oauth.serviceEndpoint(), QString("http://localhost:%1/response/1").arg(m_listenPort));
    QCOMPARE(oauth.handle(), QString("ioriayane.test"));

    {
        QSignalSpy spy(&oauth, SIGNAL(authorizationServerChanged()));
        spy.wait();
        QCOMPARE(spy.count(), 1);
    }
    QVERIFY(oauth.authorizationServer()
            == QString("http://localhost:%1/response/2").arg(m_listenPort));

    {
        QSignalSpy spy(&oauth, SIGNAL(pushedAuthorizationRequestEndpointChanged()));
        spy.wait();
        QCOMPARE(spy.count(), 1);
    }
    QVERIFY(oauth.pushedAuthorizationRequestEndpoint()
            == QString("http://localhost:%1/response/2/oauth/par").arg(m_listenPort));
    QVERIFY(oauth.authorizationEndpoint()
            == QString("http://localhost:%1/response/2/oauth/authorize").arg(m_listenPort));
    QCOMPARE(oauth.issuer(), QString("http://localhost:%1").arg(m_listenPort));
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
                 (QStringLiteral("http://localhost:") + QString::number(m_listenPort)
                  + QStringLiteral("/response/2/oauth/"
                                   "authorize?client_id=https%3A%2F%2Foauth.hagoromo.relog.tech%"
                                   "2Fclient-metadata.json&request_uri=urn%3Aietf%"
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
            << QString("http://localhost:%1/response/2/oauth/par").arg(m_listenPort)
            << QString("http://localhost:%1/response/2/oauth/token").arg(m_listenPort);
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
            QString("http://localhost:%1/response/2/oauth/token").arg(m_listenPort));
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
                QString("http://localhost:%1/response/2/oauth/token").arg(m_listenPort));
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
        oauth.setTokenEndopoint(QString("http://localhost:%1/response/2/oauth/%2")
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
            QString("http://localhost:%1/response/2/oauth/token").arg(m_listenPort));
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
    const QString plc = QString("http://localhost:%1/response/plc").arg(m_listenPort);
    const QString service = QString("http://localhost:%1/response/2").arg(m_listenPort);
    const QString pds = QString("http://localhost:%1/response/1").arg(m_listenPort);
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
    // サーバーに置くclient-metadata.jsonの原本とアプリの設定が一致していること
    QByteArray data;
    QVERIFY(SimpleHttpServer::readFile(":/client-metadata.json", data));
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

void oauth_test::test_well_known_atproto_did()
{
    AtProtocolInterface::AccountData account;
    account.service = QString("http://localhost:%1/response/3").arg(m_listenPort);

    AtProtocolInterface::WellKnownAtprotoDid well_known;
    well_known.setAccount(account);
    QSignalSpy spy(&well_known, SIGNAL(finished(bool)));
    well_known.atprotoDid();
    spy.wait();
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.takeFirst().at(0).toBool());
    QCOMPARE(well_known.did(), QString("did:plc:ipj5qejfoqu6eukvt72uhyit"));
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
