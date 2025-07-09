#include <QtTest>
#include <QCoreApplication>
#include <QDebug>

#include "tools/accountmanager.h"
#include "operation/recordoperator.h"
#include "webserver.h"

class video_teset : public QObject
{
    Q_OBJECT

public:
    video_teset();
    ~video_teset();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_postWithVideo_1();
    void test_postWithVideo_2();
    void test_postWithVideo_3();
    void test_postWithVideo_4();
    void test_postWithVideo_5();
    void test_postWithVideo_6();

private:
    WebServer m_mockServer;
    quint16 m_listenPort;
    QString m_service;
};

video_teset::video_teset()
{
    QCoreApplication::setOrganizationName(QStringLiteral("relog"));
    QCoreApplication::setApplicationName(QStringLiteral("Hagoromo_unittest"));

    m_listenPort = m_mockServer.listen(QHostAddress::LocalHost, 0);
    m_service = QString("http://localhost:%1/response").arg(m_listenPort);

    connect(&m_mockServer, &WebServer::receivedPost,
            [=](const QHttpServerRequest &request, bool &result, QString &json) {
                qDebug() << "receive POST" << request.url().path();
                QByteArray data;
                if (request.url().path().endsWith("/xrpc/app.bsky.video.uploadVideo")) {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
                    for (const auto key : request.headers().keys()) {
                        QString value = request.headers().value(key).toString();
#else
                    for(const auto &header: request.headers().toListOfPairs()){
                        QString value = header.second;
                        QString key = header.first;
#endif
                        qDebug().noquote() << "  header:" << key << value;
                        if (key == "PostFile") {
                            QVERIFY(QFile::exists(value));
                            QVERIFY(WebServer::readFile(value, data));
                            QVERIFY2(request.body().size() == data.size(),
                                     QString("%1 == %2")
                                             .arg(request.body().size(), data.size())
                                             .toLocal8Bit());
                            QVERIFY(request.body() == data);
                            break;
                        }
                    }
                }
                data.clear();
                if (WebServer::readFile(WebServer::convertResoucePath(request.url()), data)) {
                    json = QString::fromUtf8(data);
                    result = true;
                } else {
                    json = "{}";
                    result = false;
                }
                qDebug() << "  result:" << result << ", json:" << json;
                qDebug() << "  " << WebServer::convertResoucePath(request.url());
            });
}

video_teset::~video_teset() { }

void video_teset::initTestCase() { }

void video_teset::cleanupTestCase() { }

void video_teset::test_postWithVideo_1()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/limit/1", "id1", "pass1", "did:plc:ipj5qejfoqu6eukvt72uhyit_1",
            "hogehoge1.bsky.social", "email", "accessJwt", "refreshJwt", true);

    m_mockServer.reset();
    RecordOperator ope;
    ope.setVideoEndpoint(m_service + "/limit/1");
    ope.setAccount(uuid);
    {
        ope.setVideo(":/data/example_unknown.mp4");

        QSignalSpy spy(&ope, SIGNAL(finished(bool, const QString &, const QString &)));
        ope.postWithVideo();
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).typeId() == QMetaType::Bool);
        QVERIFY(arguments.at(0).toBool() == false);
    }
}

void video_teset::test_postWithVideo_2()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/limit/2", "id2", "pass2", "did:plc:ipj5qejfoqu6eukvt72uhyit_2",
            "hogehoge2.bsky.social", "email", "accessJwt", "refreshJwt", true);

    m_mockServer.reset();
    RecordOperator ope;
    ope.setVideoEndpoint(m_service + "/limit/2");
    ope.setAccount(uuid);
    {
        ope.setVideo(":/data/example01.mp4");

        QSignalSpy spy(&ope, SIGNAL(finished(bool, const QString &, const QString &)));
        ope.postWithVideo();
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).typeId() == QMetaType::Bool);
        QVERIFY(arguments.at(0).toBool() == false);
    }
}

void video_teset::test_postWithVideo_3()
{

    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/limit/3", "id3", "pass3", "did:plc:ipj5qejfoqu6eukvt72uhyit_3",
            "hogehoge3.bsky.social", "email", "accessJwt", "refreshJwt", true);

    m_mockServer.reset();
    RecordOperator ope;
    ope.setVideoEndpoint(m_service + "/limit/3");
    ope.setAccount(uuid);
    {
        ope.setVideo(":/data/example01.mp4");

        QSignalSpy spy(&ope, SIGNAL(finished(bool, const QString &, const QString &)));
        ope.postWithVideo();
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).typeId() == QMetaType::Bool);
        QVERIFY(arguments.at(0).toBool() == false);
    }
}
void video_teset::test_postWithVideo_4()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/limit/4", "id4", "pass4", "did:plc:ipj5qejfoqu6eukvt72uhyit_4",
            "hogehoge4.bsky.social", "email", "accessJwt", "refreshJwt", true);
    m_mockServer.reset();
    RecordOperator ope;
    ope.setVideoEndpoint(m_service + "/limit/4");
    ope.setAccount(uuid);
    {
        ope.setVideo(":/data/example01.mp4");

        QSignalSpy spy(&ope, SIGNAL(finished(bool, const QString &, const QString &)));
        ope.postWithVideo();
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).typeId() == QMetaType::Bool);
        QVERIFY(arguments.at(0).toBool() == false);
    }
}

void video_teset::test_postWithVideo_5()
{
    //一度アップロードして使わなかった動画を再アップロードしたとき？

    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/limit/5", "id5", "pass5", "did:plc:ipj5qejfoqu6eukvt72uhyit_5",
            "hogehoge5.bsky.social", "email", "accessJwt", "refreshJwt", true);

    m_mockServer.reset();
    RecordOperator ope;
    ope.setVideoEndpoint(m_service + "/limit/5");
    ope.setAccount(uuid);
    {
        ope.setText("upload video 5");
        ope.setVideo(":/data/example01.mp4");

        QSignalSpy spy(&ope, SIGNAL(finished(bool, const QString &, const QString &)));
        ope.postWithVideo();
        spy.wait(30 * 1000);
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).typeId() == QMetaType::Bool);
        QVERIFY(arguments.at(0).toBool() == true);
    }
}

void video_teset::test_postWithVideo_6()
{

    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/limit/6", "id6", "pass6", "did:plc:ipj5qejfoqu6eukvt72uhyit_6",
            "hogehoge6.bsky.social", "email", "accessJwt", "refreshJwt", true);

    m_mockServer.reset();
    RecordOperator ope;
    // ope.clear();
    ope.setVideoEndpoint(m_service + "/limit/6");
    ope.setAccount(uuid);
    {
        ope.setText("upload video 6");
        ope.setVideo(":/data/example01.mp4");

        QSignalSpy spy(&ope, SIGNAL(finished(bool, const QString &, const QString &)));
        ope.postWithVideo();
        spy.wait(90 * 1000);
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).typeId() == QMetaType::Bool);
        QVERIFY(arguments.at(0).toBool() == true);
    }
}

QTEST_MAIN(video_teset)

#include "tst_video_test.moc"
