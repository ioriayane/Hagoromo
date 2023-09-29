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
    void test_get();
    void test_post();

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
                QByteArray data;
                if (request.url().path() == "/response/xrpc/com.atproto.repo.uploadBlob") {
                    for (const auto key : request.headers().keys()) {
                        QString value = request.headers().value(key).toString();
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
            });
}

http_test::~http_test() { }

void http_test::initTestCase() { }

void http_test::cleanupTestCase() { }

void http_test::test_get()
{
    HttpAccessManager manager;
    QNetworkRequest request(m_service + "/xrpc/app.bsky.feed.getTimeline");
    QByteArray expect_data;
    QString expect_str;
    {
        QSignalSpy spy(&manager, SIGNAL(finished(bool)));
        HttpReply *reply = manager.get(request);
        spy.wait();
        QVERIFY(spy.count() == 1);
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());

        QVERIFY(WebServer::readFile(":/response/xrpc/app.bsky.feed.getTimeline", expect_data));
        expect_str = QString::fromUtf8(expect_data);
        QVERIFY(expect_str.size() == reply->body().size());
        QVERIFY(expect_str == reply->body());

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

        QVERIFY(WebServer::readFile(":/response/xrpc/app.bsky.actor.getPreferences", expect_data));
        expect_str = QString::fromUtf8(expect_data);
        QVERIFY(expect_str.size() == reply->body().size());
        QVERIFY(QString::fromUtf8(expect_data) == reply->body());
    }
}

void http_test::test_post()
{
    HttpAccessManager manager;
    QNetworkRequest request(m_service + "/xrpc/com.atproto.repo.createRecord");
    QByteArray expect_data;
    QString expect_str;

    {
        request.setUrl(m_service + "/xrpc/com.atproto.repo.createRecord");
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        HttpReply *reply = manager.post(request, QString("{\"create\": \"record\"}").toUtf8());
        QSignalSpy spy(reply, SIGNAL(finished(bool)));
        spy.wait();
        QVERIFY(spy.count() == 1);
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());

        QVERIFY(WebServer::readFile(":/response/xrpc/com.atproto.repo.createRecord", expect_data));
        expect_str = QString::fromUtf8(expect_data);
        QVERIFY(expect_str.size() == reply->body().size());
        QVERIFY(QString::fromUtf8(expect_data) == reply->body());
    }

    {
        QString data_path = ":/data/images/file01.png";
        QByteArray data;
        QMimeDatabase mime;

        QVERIFY(WebServer::readFile(data_path, data));
        request.setUrl(m_service + "/xrpc/com.atproto.repo.uploadBlob");
        request.setHeader(QNetworkRequest::ContentTypeHeader,
                          mime.mimeTypeForFile(data_path).name());
        request.setRawHeader("PostFile", data_path.toLocal8Bit());
        HttpReply *reply = manager.post(request, data);
        QSignalSpy spy(reply, SIGNAL(finished(bool)));
        spy.wait();
        QVERIFY(spy.count() == 1);
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());

        QVERIFY(WebServer::readFile(":/response/xrpc/com.atproto.repo.uploadBlob", expect_data));
        expect_str = QString::fromUtf8(expect_data);
        QVERIFY(expect_str.size() == reply->body().size());
        QVERIFY(QString::fromUtf8(expect_data) == reply->body());
    }
}

QTEST_MAIN(http_test)

#include "tst_http_test.moc"
