#include <QtTest>
#include <QCoreApplication>

#include "webserver.h"
#include "atprotocol/com/atproto/server/comatprotoservercreatesession.h"

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

QTEST_MAIN(atprotocol_test)

#include "tst_atprotocol_test.moc"
