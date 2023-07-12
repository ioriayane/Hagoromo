#include <QtTest>
#include <QCoreApplication>
#include <QTemporaryFile>

#include "webserver.h"
#include "atprotocol/com/atproto/server/comatprotoservercreatesession.h"
#include "tools/opengraphprotocol.h"

class atprotocol_test : public QObject
{
    Q_OBJECT

public:
    atprotocol_test();
    ~atprotocol_test();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_ComAtprotoServerCreateSession();
    void test_OpenGraphProtocol();

private:
    WebServer m_mockServer;
    quint16 m_listenPort;
    QString m_service;
};

atprotocol_test::atprotocol_test()
{
    m_listenPort = m_mockServer.listen(QHostAddress::LocalHost, 0);
    m_service = QString("http://localhost:%1/response").arg(m_listenPort);

    connect(&m_mockServer, &WebServer::receivedPost,
            [=](const QHttpServerRequest &request, bool &result, QString &json) {
                QFile *file = new QFile(":" + request.url().path());
                if (file->open(QFile::ReadOnly)) {
                    json = file->readAll();
                } else {
                    json = "{}";
                }
                result = true;
            });
}

atprotocol_test::~atprotocol_test() { }

void atprotocol_test::initTestCase()
{
    QVERIFY(m_listenPort != 0);
}

void atprotocol_test::cleanupTestCase() { }

void atprotocol_test::test_ComAtprotoServerCreateSession()
{
    AtProtocolInterface::ComAtprotoServerCreateSession session;
    session.setService(m_service);

    QSignalSpy spy(&session, SIGNAL(finished(bool)));
    session.create("hoge", "fuga");
    spy.wait();
    QVERIFY(spy.count() == 1);

    QList<QVariant> arguments = spy.takeFirst();
    QVERIFY(arguments.at(0).toBool());

    QVERIFY(session.did() == "did:plc:ipj5qejfoqu6eukvt72uhyit");
    QVERIFY(session.handle() == "ioriayane.relog.tech");
    QVERIFY(session.email() == "iori.ayane@gmail.com");
    QVERIFY(session.accessJwt() == "hoge hoge accessJwt");
    QVERIFY(session.refreshJwt() == "hoge hoge refreshJwt");
}

void atprotocol_test::test_OpenGraphProtocol()
{
    OpenGraphProtocol ogp;
    {
        QSignalSpy spy(&ogp, SIGNAL(finished(bool)));
        ogp.getData(m_service + "/ogp/file1.html");
        spy.wait();
        QVERIFY(spy.count() == 1);

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());

        QVERIFY(ogp.uri() == "http://localhost/response/ogp/file1.html");
        QVERIFY(ogp.title() == "file1 title");
        QVERIFY(ogp.description() == "file1 description");
        QVERIFY(ogp.thumb() == "http://localhost:%1/response/ogp/images/file1.jpg");
    }
    {
        QTemporaryFile temp_file;
        temp_file.open();
        ogp.setThumb(ogp.thumb().replace("%1", QString::number(m_listenPort)));

        QSignalSpy spy(&ogp, SIGNAL(finishedDownload(bool)));
        ogp.downloadThumb(temp_file.fileName());
        spy.wait();
        QVERIFY(spy.count() == 1);

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());

        QVERIFY(QFile::exists(temp_file.fileName()));
        QImage img(temp_file.fileName());
        QVERIFY(!img.isNull());
        QVERIFY(img.width() == 664);
        QVERIFY(img.height() == 257);
    }

    {
        QSignalSpy spy(&ogp, SIGNAL(finished(bool)));
        ogp.getData(m_service + "/ogp/file2.html");
        spy.wait();
        QVERIFY(spy.count() == 1);

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());

        QVERIFY(ogp.uri() == "http://localhost/response/ogp/file2.html");
        QVERIFY2(ogp.title()
                         == QString("file2 ")
                                    .append(QChar(0x30bf))
                                    .append(QChar(0x30a4))
                                    .append(QChar(0x30c8))
                                    .append(QChar(0x30eb)),
                 ogp.title().toLocal8Bit());
        QVERIFY2(ogp.description() == QString("file2 ").append(QChar(0x8a73)).append(QChar(0x7d30)),
                 ogp.description().toLocal8Bit());
        QVERIFY(ogp.thumb() == "http://localhost:%1/response/ogp/images/file2.gif");
    }
    {
        QTemporaryFile temp_file;
        temp_file.open();

        ogp.setThumb(ogp.thumb().replace("%1", QString::number(m_listenPort)));

        QSignalSpy spy(&ogp, SIGNAL(finishedDownload(bool)));
        ogp.downloadThumb(temp_file.fileName());
        spy.wait();
        QVERIFY(spy.count() == 1);

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());

        QVERIFY(QFile::exists(temp_file.fileName()));
        QImage img(temp_file.fileName());
        QVERIFY(!img.isNull());
        QVERIFY(img.width() == 664);
        QVERIFY(img.height() == 257);
    }

    {
        QSignalSpy spy(&ogp, SIGNAL(finished(bool)));
        ogp.getData(m_service + "/ogp/file3.html");
        spy.wait();
        QVERIFY(spy.count() == 1);

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());

        QVERIFY(ogp.uri() == "http://localhost/response/ogp/file3.html");
        QVERIFY2(ogp.title()
                         == QString("file3 ")
                                    .append(QChar(0x30bf))
                                    .append(QChar(0x30a4))
                                    .append(QChar(0x30c8))
                                    .append(QChar(0x30eb)),
                 ogp.title().toLocal8Bit());
        QVERIFY2(ogp.description() == QString("file3 ").append(QChar(0x8a73)).append(QChar(0x7d30)),
                 ogp.description().toLocal8Bit());
        QVERIFY(ogp.thumb() == "http://localhost:%1/response/ogp/images/file3.png");
    }
    {
        QTemporaryFile temp_file;
        temp_file.open();

        ogp.setThumb(ogp.thumb().replace("%1", QString::number(m_listenPort)));

        QSignalSpy spy(&ogp, SIGNAL(finishedDownload(bool)));
        ogp.downloadThumb(temp_file.fileName());
        spy.wait();
        QVERIFY(spy.count() == 1);

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());

        QVERIFY(QFile::exists(temp_file.fileName()));
        QImage img(temp_file.fileName());
        QVERIFY(!img.isNull());
        QVERIFY(img.width() == 664);
        QVERIFY(img.height() == 257);
    }

    {
        QSignalSpy spy(&ogp, SIGNAL(finished(bool)));
        ogp.getData(m_service + "/ogp/file4.html");
        spy.wait();
        QVERIFY(spy.count() == 1);

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());

        QVERIFY2(
                ogp.uri()
                        == QString("http://localhost:%1/response/ogp/file4.html").arg(m_listenPort),
                ogp.uri().toLocal8Bit());
        QVERIFY2(ogp.title()
                         == QString("file4 ")
                                    .append(QChar(0x30bf))
                                    .append(QChar(0x30a4))
                                    .append(QChar(0x30c8))
                                    .append(QChar(0x30eb)),
                 ogp.title().toLocal8Bit());
        QVERIFY2(ogp.description() == QString("file4 ").append(QChar(0x8a73)).append(QChar(0x7d30)),
                 ogp.description().toLocal8Bit());
        QVERIFY(ogp.thumb() == "http://localhost:%1/response/ogp/images/file3.png");
    }
    {
        QTemporaryFile temp_file;
        temp_file.open();

        ogp.setThumb(ogp.thumb().replace("%1", QString::number(m_listenPort)));

        QSignalSpy spy(&ogp, SIGNAL(finishedDownload(bool)));
        ogp.downloadThumb(temp_file.fileName());
        spy.wait();
        QVERIFY(spy.count() == 1);

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());

        QVERIFY(QFile::exists(temp_file.fileName()));
        QImage img(temp_file.fileName());
        QVERIFY(!img.isNull());
        QVERIFY(img.width() == 664);
        QVERIFY(img.height() == 257);
    }
}

QTEST_MAIN(atprotocol_test)

#include "tst_atprotocol_test.moc"
