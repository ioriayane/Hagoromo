#include <QtTest>
#include <QCoreApplication>
#include <QDebug>

#include "webserver.h"
#include "http/httpaccessmanager.h"

class http_test : public QObject
{
    Q_OBJECT

public:
    http_test();
    ~http_test();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_case1();

private:
    WebServer m_mockServer;
    quint16 m_listenPort;
    QString m_service;
};

http_test::http_test()
{
    QCoreApplication::setOrganizationName(QStringLiteral("relog"));
    QCoreApplication::setApplicationName(QStringLiteral("Hagoromo"));

    m_listenPort = m_mockServer.listen(QHostAddress::LocalHost, 0);
    m_service = QString("http://localhost:%1/response").arg(m_listenPort);

    connect(&m_mockServer, &WebServer::receivedPost,
            [=](const QHttpServerRequest &request, bool &result, QString &json) {
                qDebug() << "receive POST" << request.url().path();
                json = "{}";
                result = true;
            });
}

http_test::~http_test() { }

void http_test::initTestCase() { }

void http_test::cleanupTestCase() { }

void http_test::test_case1()
{
    qDebug() << m_service;

    HttpAccessManager manager;
    QNetworkRequest request(m_service + "/xrpc/app.bsky.feed.getTimeline");

    {
        QSignalSpy spy(&manager, SIGNAL(finished(bool)));
        HttpReply *reply = manager.get(request);
        spy.wait();
        QVERIFY(spy.count() == 1);
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
        qDebug().noquote() << reply->body().left(40);
        reply->deleteLater();
    }

    {
        request.setUrl(m_service + "/xrpc/app.bsky.actor.getPreferences");
        HttpReply *reply = manager.get(request);
        QSignalSpy spy(reply, SIGNAL(finished(bool)));
        spy.wait();
        QVERIFY(spy.count() == 1);
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
        qDebug().noquote() << reply->body().left(40);
    }

    {
        request.setUrl(m_service + "/xrpc/com.atproto.repo.createRecord");
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        HttpReply *reply = manager.post(request, QString("{\"create\": \"record\"}").toUtf8());
        QSignalSpy spy(reply, SIGNAL(finished(bool)));
        spy.wait();
        QVERIFY(spy.count() == 1);
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
        qDebug().noquote() << reply->body().left(40);
    }
}

QTEST_MAIN(http_test)

#include "tst_http_test.moc"
