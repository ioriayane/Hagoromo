#include <QtTest>
#include <QCoreApplication>

#include "tools/authorization.h"

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
};

oauth_test::oauth_test() { }

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
    qDebug() << "ParPlayload" << oauth.ParPlayload();

    oauth.par();
}

void oauth_test::test_oauth_server()
{

    Authorization oauth;

    {
        QSignalSpy spy(&oauth, SIGNAL(finished(bool)));
        oauth.startRedirectServer();
        spy.wait(60 * 1000);
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }
}

QTEST_MAIN(oauth_test)

#include "tst_oauth_test.moc"
