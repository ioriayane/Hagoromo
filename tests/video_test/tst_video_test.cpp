#include <QtTest>
#include <QCoreApplication>
#include <QDebug>

#include "operation/recordoperator.h"
#include "webserver.h"
// #include "http/httpaccessmanager.h"

class video_teset : public QObject
{
    Q_OBJECT

public:
    video_teset();
    ~video_teset();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_get();

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
                if (request.url().path() == "/response/xrpc/com.atproto.repo.uploadBlob") {
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
            });
}

video_teset::~video_teset() { }

void video_teset::initTestCase() { }

void video_teset::cleanupTestCase() { }

void video_teset::test_get()
{
    RecordOperator ope;

    {
        QSignalSpy spy(&ope, SIGNAL(finished(bool, const QString &, const QString &)));
        ope.postWithVideo();
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).typeId() == QMetaType::Bool);
        QVERIFY(arguments.at(0).toBool() == true);
    }
}

QTEST_MAIN(video_teset)

#include "tst_video_test.moc"
