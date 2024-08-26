#include <QtTest>
#include <QCoreApplication>

#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/bn.h>

#include "tools/authorization.h"
#include "tools/jsonwebtoken.h"
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
    void test_jwt();

private:
    SimpleHttpServer m_server;
    quint16 m_listenPort;

    void verify_jwt(const QByteArray &jwt);
};

oauth_test::oauth_test()
{

    m_listenPort = m_server.listen(QHostAddress::LocalHost, 0);
    connect(&m_server, &SimpleHttpServer::received, this,
            [=](const QHttpServerRequest &request, bool &result, QByteArray &data,
                QByteArray &mime_type) {
                //
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

void oauth_test::test_jwt()
{
    QByteArray jwt = JsonWebToken::generate("https://hoge");

    qDebug().noquote() << jwt;

    verify_jwt(jwt);

    QVERIFY(true);
}

void oauth_test::verify_jwt(const QByteArray &jwt)
{
    const QByteArrayList jwt_parts = jwt.split('.');
    QVERIFY2(jwt_parts.length() == 3, jwt);

    QByteArray message = jwt_parts[0] + '.' + jwt_parts[1];
    QByteArray sig = QByteArray::fromBase64(
            jwt_parts.last(), QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
    QVERIFY2(sig.length() == 64, QString::number(sig.length()).toLocal8Bit());
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

    FILE *fp = fopen("c:\\temp\\public_key.pem", "r");
    QVERIFY(fp != NULL);
    EVP_PKEY *pkey = PEM_read_PUBKEY(fp, nullptr, nullptr, nullptr);
    fclose(fp);
    QVERIFY(pkey != NULL);

    // ECDSA署名の検証
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    QVERIFY(EVP_DigestVerifyInit(mdctx, nullptr, EVP_sha256(), nullptr, pkey) > 0);
    QVERIFY(EVP_DigestVerifyUpdate(mdctx, message.constData(), message.length()) > 0);
    QVERIFY(EVP_DigestVerifyFinal(mdctx, der_sig, der_sig_len) > 0);

    EVP_MD_CTX_free(mdctx);
    EVP_PKEY_free(pkey);

    BN_free(ec_sig_r);
    BN_free(ec_sig_s);
    OPENSSL_free(der_sig);
    ECDSA_SIG_free(ec_sig);
}

QTEST_MAIN(oauth_test)

#include "tst_oauth_test.moc"
