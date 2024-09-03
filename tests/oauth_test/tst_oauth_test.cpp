#include <QtTest>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>

#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/bn.h>

#include "tools/authorization.h"
#include "tools/jsonwebtoken.h"
#include "tools/es256.h"
#include "http/simplehttpserver.h"

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
    void test_jwt();
    void test_es256();

private:
    SimpleHttpServer m_server;
    quint16 m_listenPort;

    void verify_jwt(const QByteArray &jwt, EVP_PKEY *pkey);
};

oauth_test::oauth_test()
{
    QCoreApplication::setOrganizationName(QStringLiteral("relog"));
    QCoreApplication::setApplicationName(QStringLiteral("Hagoromo"));

    m_listenPort = m_server.listen(QHostAddress::LocalHost, 0);
    connect(&m_server, &SimpleHttpServer::received, this,
            [=](const QHttpServerRequest &request, bool &result, QByteArray &data,
                QByteArray &mime_type) {
                //
                qDebug().noquote() << request.url();
                QString path = SimpleHttpServer::convertResoucePath(request.url());
                qDebug().noquote() << " res path =" << path;
                if (!QFile::exists(path)) {
                    result = false;
                } else {
                    mime_type = "application/json";
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
    qDebug() << sizeof(base) << base_ba.size()
             << base_ba.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);

    QString msg;
    QByteArray sha256 = QCryptographicHash::hash(
            base_ba.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals),
            QCryptographicHash::Sha256);
    for (const auto s : sha256) {
        msg += QString::number(static_cast<unsigned char>(s)) + ", ";
    }
    qDebug() << msg;
    qDebug() << sha256.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);

    Authorization oauth;
    oauth.makeCodeChallenge();
    qDebug() << "codeChallenge" << oauth.codeChallenge();
    qDebug() << "codeVerifier" << oauth.codeVerifier();

    // QVERIFY(oauth.codeChallenge()
    //         == base_ba.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
    // QVERIFY(oauth.codeVerifier()
    //         == sha256.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));

    oauth.makeParPayload();
    qDebug() << "ParPlayload" << oauth.ParPayload();

    // oauth.par();
}

void oauth_test::test_oauth_server()
{

    Authorization oauth;

    // {
    //     QSignalSpy spy(&oauth, SIGNAL(finished(bool)));
    //     oauth.startRedirectServer();
    //     spy.wait(60 * 1000);
    //     QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    //     QList<QVariant> arguments = spy.takeFirst();
    //     QVERIFY(arguments.at(0).toBool());
    // }
}

void oauth_test::test_oauth()
{
    // response/1 : pds
    // response/2 : entry-way

    Authorization oauth;
    QString pds = QString("http://localhost:%1/response/2").arg(m_listenPort);
    QString handle = "ioriayane.relog.tech";

    oauth.reset();
    {
        QSignalSpy spy(&oauth, SIGNAL(serviceEndpointChanged()));
        oauth.start(pds, handle);
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }
    QVERIFY(oauth.serviceEndpoint() == QString("http://localhost:%1/response/1").arg(m_listenPort));

    {
        QSignalSpy spy(&oauth, SIGNAL(authorizationServerChanged()));
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }
    QVERIFY(oauth.authorizationServer()
            == QString("http://localhost:%1/response/2").arg(m_listenPort));

    {
        QSignalSpy spy(&oauth, SIGNAL(pushedAuthorizationRequestEndpointChanged()));
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }
    QVERIFY(oauth.pushedAuthorizationRequestEndpoint()
            == QString("http://localhost:%1/response/2/oauth/par").arg(m_listenPort));
    QVERIFY(oauth.authorizationEndpoint()
            == QString("http://localhost:%1/response/2/oauth/authorize").arg(m_listenPort));

    //
    {
        QSignalSpy spy(&oauth, SIGNAL(madeRedirectUrl(const QString &)));
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY2(arguments.at(0).toString()
                         == "https://bsky.social/oauth/"
                            "authorize?client_id=http%3A%2F%2Flocalhost%2Ftech%2Frelog%2Fhagoromo%"
                            "3Fredirect_uri%3Dhttp%253A%252F%252F127.0.0.1%253A8080%252Ftech%"
                            "252Frelog%"
                            "252Fhagoromo%252Foauth-callback&request_uri=urn%3Aietf%3Aparams%"
                            "3Aoauth%"
                            "3Arequest_uri%3Areq-05650c01604941dc674f0af9cb032aca",
                 arguments.at(0).toString().toLocal8Bit());
    }

    // {
    //     QSignalSpy spy(&oauth, SIGNAL(finished(bool)));
    //     spy.wait();
    //     QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    //     QList<QVariant> arguments = spy.takeFirst();
    //     QVERIFY(arguments.at(0).toBool());
    // }
}

void oauth_test::test_jwt()
{
    QByteArray jwt = JsonWebToken::generate("https://hoge");

    qDebug().noquote() << jwt;

    verify_jwt(jwt, Es256::getInstance()->pKey());

    QVERIFY(true);
}

void oauth_test::test_es256()
{
    QString private_key_path =
            QString("%1/%2/%3%4/private_key.pem")
                    .arg(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation))
                    .arg(QCoreApplication::organizationName())
                    .arg(QCoreApplication::applicationName())
                    .arg(QStringLiteral("_unittest"));
    QFile::remove(private_key_path);
    Es256::getInstance()->clear();

    {
        QString message = "header.payload";
        QByteArray sign = Es256::getInstance()->sign(message.toUtf8());
        QByteArray jwt = message.toUtf8() + '.'
                + sign.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);

        QVERIFY(QFile::exists(private_key_path));
        verify_jwt(jwt, Es256::getInstance()->pKey());
    }
    QFile::remove(private_key_path);
    QVERIFY(!QFile::exists(private_key_path));
    {
        QString message = "header2.payload2";
        QByteArray sign = Es256::getInstance()->sign(message.toUtf8());
        QByteArray jwt = message.toUtf8() + '.'
                + sign.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);

        verify_jwt(jwt, Es256::getInstance()->pKey());
    }
}

void oauth_test::verify_jwt(const QByteArray &jwt, EVP_PKEY *pkey)
{
    EC_KEY *ec_key = nullptr;

    const QByteArrayList jwt_parts = jwt.split('.');
    QVERIFY2(jwt_parts.length() == 3, jwt);

    QByteArray message = jwt_parts[0] + '.' + jwt_parts[1];
    QByteArray sig = QByteArray::fromBase64(
            jwt_parts.last(), QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
    QVERIFY2(sig.length() == 64, QString::number(sig.length()).toLocal8Bit());
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
    QVERIFY(ECDSA_SIG_set0(ec_sig, ec_sig_r, ec_sig_s) == 1);

    unsigned char *der_sig = NULL;
    int der_sig_len = i2d_ECDSA_SIG(ec_sig, &der_sig);
    QVERIFY(der_sig_len > 0);

    // ECDSA署名の検証
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    QVERIFY(pkey != nullptr);
    QVERIFY(EVP_DigestVerifyInit(mdctx, nullptr, EVP_sha256(), nullptr, pkey) > 0);
    QVERIFY(EVP_DigestVerifyUpdate(mdctx, message.constData(), message.length()) > 0);
    QVERIFY(EVP_DigestVerifyFinal(mdctx, der_sig, der_sig_len) > 0);
    EVP_MD_CTX_free(mdctx);

    OPENSSL_free(der_sig);
    ECDSA_SIG_free(ec_sig);
    BN_free(ec_sig_s);
    BN_free(ec_sig_r);

    if (use_jwk) {
        EVP_PKEY_free(pkey);
        // EC_KEY_free(ec_key); EVP_PKEY_freeで解放される
    }
}

QTEST_MAIN(oauth_test)

#include "tst_oauth_test.moc"
