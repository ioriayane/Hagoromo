#include <QtTest>
#include <QCoreApplication>

#include "webserver.h"
#include "chat/chatlistmodel.h"
#include "chat/chatmessagelistmodel.h"

class chat_test : public QObject
{
    Q_OBJECT

public:
    chat_test();
    ~chat_test();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_ChatListModel();
    void test_ChatMessageListModel();

private:
    WebServer m_mockServer;
    quint16 m_listenPort;
    QString m_service;
};

chat_test::chat_test()
{
    QCoreApplication::setOrganizationName(QStringLiteral("relog"));
    QCoreApplication::setApplicationName(QStringLiteral("Hagoromo"));

    m_listenPort = m_mockServer.listen(QHostAddress::LocalHost, 0);
    m_service = QString("http://localhost:%1/response").arg(m_listenPort);

    connect(&m_mockServer, &WebServer::receivedPost,
            [=](const QHttpServerRequest &request, bool &result, QString &json) {
                if (request.url().path().endsWith("/xrpc/com.atproto.server.refreshSession")) {
                    QFileInfo file_info(request.url().path());
                    QString path = ":" + file_info.filePath();
                    QFile file(path);
                    if (file.open(QFile::ReadOnly)) {
                        json = file.readAll();
                        file.close();
                        result = true;
                    } else {
                        json = "{}";
                        result = false;
                    }
                } else if (request.url().path().endsWith(
                                   "/xrpc/app.bsky.notification.updateSeen")) {
                    json = "{}";
                    result = true;
                } else {
                    json = "{}";
                    result = false;
                    QVERIFY(false);
                }
            });
}

chat_test::~chat_test() { }

void chat_test::initTestCase()
{
    QVERIFY(m_listenPort != 0);
}

void chat_test::cleanupTestCase() { }

void chat_test::test_ChatListModel()
{
    int i = 0;
    ChatListModel model;
    model.setAccount(m_service + "/list", "did:plc:ipj5qejfoqu6eukvt72uhyit", "handle", "email",
                     "accessJwt", "refreshJwt");
    model.setServiceEndpoint(m_service + "/list");

    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait();
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }

    QVERIFY2(model.rowCount() == 2, QString::number(model.rowCount()).toLocal8Bit());

    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait();
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }

    QVERIFY2(model.rowCount() == 2, QString::number(model.rowCount()).toLocal8Bit());

    i = 0;
    QVERIFY2(model.item(i, ChatListModel::IdRole).toString() == "c2222",
             model.item(i, ChatListModel::IdRole).toString().toLocal8Bit());

    i = 1;
    QVERIFY2(model.item(i, ChatListModel::IdRole).toString() == "c1111",
             model.item(i, ChatListModel::IdRole).toString().toLocal8Bit());

    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getNext();
        spy.wait();
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }

    QVERIFY2(model.rowCount() == 4, QString::number(model.rowCount()).toLocal8Bit());

    i = 0;
    QVERIFY2(model.item(i, ChatListModel::IdRole).toString() == "c2222",
             model.item(i, ChatListModel::IdRole).toString().toLocal8Bit());

    i = 1;
    QVERIFY2(model.item(i, ChatListModel::IdRole).toString() == "c1111",
             model.item(i, ChatListModel::IdRole).toString().toLocal8Bit());

    i = 2;
    QVERIFY2(model.item(i, ChatListModel::IdRole).toString() == "c4444",
             model.item(i, ChatListModel::IdRole).toString().toLocal8Bit());

    i = 3;
    QVERIFY2(model.item(i, ChatListModel::IdRole).toString() == "c3333",
             model.item(i, ChatListModel::IdRole).toString().toLocal8Bit());
}

void chat_test::test_ChatMessageListModel()
{
    ChatMessageListModel model;
    int i = 0;

    model.setAccount(m_service + "/message/1", "did:plc:ipj5qejfoqu6eukvt72uhyit", "handle",
                     "email", "accessJwt", "refreshJwt");
    model.setServiceEndpoint(m_service + "/message/1");

    model.setConvoId("3ksrqt7eebs2b");

    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait();
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }

    QVERIFY2(model.rowCount() == 5, QString::number(model.rowCount()).toLocal8Bit());
    i = 0;
    QVERIFY2(model.item(i, ChatMessageListModel::IdRole).toString() == "3ksyzepxa2s2m",
             model.item(i, ChatMessageListModel::IdRole).toString().toLocal8Bit());
    i = 1;
    QVERIFY2(model.item(i, ChatMessageListModel::IdRole).toString() == "3ksyzejyngk2s",
             model.item(i, ChatMessageListModel::IdRole).toString().toLocal8Bit());
    i = 2;
    QVERIFY2(model.item(i, ChatMessageListModel::IdRole).toString() == "3ksyzdsn7hk2m",
             model.item(i, ChatMessageListModel::IdRole).toString().toLocal8Bit());
    i = 3;
    QVERIFY2(model.item(i, ChatMessageListModel::IdRole).toString() == "3ksyzdmpzqs2s",
             model.item(i, ChatMessageListModel::IdRole).toString().toLocal8Bit());
    i = 4;
    QVERIFY2(model.item(i, ChatMessageListModel::IdRole).toString() == "3ksyzdesy3s2m",
             model.item(i, ChatMessageListModel::IdRole).toString().toLocal8Bit());

    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait();
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }

    QVERIFY2(model.rowCount() == 5, QString::number(model.rowCount()).toLocal8Bit());
    i = 0;
    QVERIFY2(model.item(i, ChatMessageListModel::IdRole).toString() == "3ksyzepxa2s2m",
             model.item(i, ChatMessageListModel::IdRole).toString().toLocal8Bit());
    i = 1;
    QVERIFY2(model.item(i, ChatMessageListModel::IdRole).toString() == "3ksyzejyngk2s",
             model.item(i, ChatMessageListModel::IdRole).toString().toLocal8Bit());
    i = 2;
    QVERIFY2(model.item(i, ChatMessageListModel::IdRole).toString() == "3ksyzdsn7hk2m",
             model.item(i, ChatMessageListModel::IdRole).toString().toLocal8Bit());
    i = 3;
    QVERIFY2(model.item(i, ChatMessageListModel::IdRole).toString() == "3ksyzdmpzqs2s",
             model.item(i, ChatMessageListModel::IdRole).toString().toLocal8Bit());
    i = 4;
    QVERIFY2(model.item(i, ChatMessageListModel::IdRole).toString() == "3ksyzdesy3s2m",
             model.item(i, ChatMessageListModel::IdRole).toString().toLocal8Bit());

    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getNext();
        spy.wait();
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }

    QVERIFY2(model.rowCount() == 10, QString::number(model.rowCount()).toLocal8Bit());
    i = 0;
    QVERIFY2(model.item(i, ChatMessageListModel::IdRole).toString() == "3ksyzepxa2s2m",
             model.item(i, ChatMessageListModel::IdRole).toString().toLocal8Bit());
    i = 1;
    QVERIFY2(model.item(i, ChatMessageListModel::IdRole).toString() == "3ksyzejyngk2s",
             model.item(i, ChatMessageListModel::IdRole).toString().toLocal8Bit());
    i = 2;
    QVERIFY2(model.item(i, ChatMessageListModel::IdRole).toString() == "3ksyzdsn7hk2m",
             model.item(i, ChatMessageListModel::IdRole).toString().toLocal8Bit());
    i = 3;
    QVERIFY2(model.item(i, ChatMessageListModel::IdRole).toString() == "3ksyzdmpzqs2s",
             model.item(i, ChatMessageListModel::IdRole).toString().toLocal8Bit());
    i = 4;
    QVERIFY2(model.item(i, ChatMessageListModel::IdRole).toString() == "3ksyzdesy3s2m",
             model.item(i, ChatMessageListModel::IdRole).toString().toLocal8Bit());
    i = 5;
    QVERIFY2(model.item(i, ChatMessageListModel::IdRole).toString() == "3ksyzd6vyak2s",
             model.item(i, ChatMessageListModel::IdRole).toString().toLocal8Bit());
    i = 6;
    QVERIFY2(model.item(i, ChatMessageListModel::IdRole).toString() == "3ksyzcwz7es2s",
             model.item(i, ChatMessageListModel::IdRole).toString().toLocal8Bit());
    i = 7;
    QVERIFY2(model.item(i, ChatMessageListModel::IdRole).toString() == "3ksyzbti4ns2k",
             model.item(i, ChatMessageListModel::IdRole).toString().toLocal8Bit());
    i = 8;
    QVERIFY2(model.item(i, ChatMessageListModel::IdRole).toString() == "3ksyzbmczm22s",
             model.item(i, ChatMessageListModel::IdRole).toString().toLocal8Bit());
    i = 9;
    QVERIFY2(model.item(i, ChatMessageListModel::IdRole).toString() == "3ksyzbg7zek2m",
             model.item(i, ChatMessageListModel::IdRole).toString().toLocal8Bit());
}

QTEST_MAIN(chat_test)

#include "tst_chat_test.moc"
