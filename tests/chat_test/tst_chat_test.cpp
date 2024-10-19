#include <QtTest>
#include <QCoreApplication>

#include "webserver.h"
#include "chat/chatlistmodel.h"
#include "chat/chatmessagelistmodel.h"
#include "tools/chatlogsubscriber.h"
#include "tools/accountmanager.h"

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
    void test_ChatMessageListModelByMembers();

    void test_ChatLogSubscriber();

private:
    WebServer m_mockServer;
    quint16 m_listenPort;
    QString m_service;
};

chat_test::chat_test()
{
    QCoreApplication::setOrganizationName(QStringLiteral("relog"));
    QCoreApplication::setApplicationName(QStringLiteral("Hagoromo_unittest"));

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
                } else if (request.url().path().endsWith("/xrpc/chat.bsky.convo.updateRead")) {
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

    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/list", "id", "pass", "did:plc:ipj5qejfoqu6eukvt72uhyit",
            "handle", "email", "accessJwt", "refreshJwt", true);
    model.setAccount(uuid);
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

    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/message/1", "id", "pass", "did:plc:ipj5qejfoqu6eukvt72uhyit",
            "handle", "email", "accessJwt", "refreshJwt", true);
    model.setAccount(uuid);
    model.setServiceEndpoint(m_service + "/message/1");

    model.setConvoId("3ksrqt7eebs2b");
    model.setAutoLoading(false);

    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait(10 * 1000);
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

    // {
    //     QSignalSpy spy(&model, SIGNAL(runningChanged()));
    //     model.getLatest();
    //     spy.wait();
    //     QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    // }

    // QVERIFY2(model.rowCount() == 15, QString::number(model.rowCount()).toLocal8Bit());

    // i = 0;
    // QVERIFY2(model.item(i, ChatMessageListModel::IdRole).toString() == "3kt5qzdatpk2r",
    //          model.item(i, ChatMessageListModel::IdRole).toString().toLocal8Bit());
    // i = 1;
    // QVERIFY2(model.item(i, ChatMessageListModel::IdRole).toString() == "3kt5qz2u7yw2q",
    //          model.item(i, ChatMessageListModel::IdRole).toString().toLocal8Bit());
    // i = 2;
    // QVERIFY2(model.item(i, ChatMessageListModel::IdRole).toString() == "3kt5qysohzt2n",
    //          model.item(i, ChatMessageListModel::IdRole).toString().toLocal8Bit());
    // i = 3;
    // QVERIFY2(model.item(i, ChatMessageListModel::IdRole).toString() == "3kt5qy5yzzd2a",
    //          model.item(i, ChatMessageListModel::IdRole).toString().toLocal8Bit());
    // i = 4;
    // QVERIFY2(model.item(i, ChatMessageListModel::IdRole).toString() == "3kt5o6y7wmr2r",
    //          model.item(i, ChatMessageListModel::IdRole).toString().toLocal8Bit());
}

void chat_test::test_ChatMessageListModelByMembers()
{
    ChatMessageListModel model;
    int i = 0;

    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/message/2", "id", "pass", "did:plc:ipj5qejfoqu6eukvt72uhyit",
            "handle", "email", "accessJwt", "refreshJwt", true);
    model.setAccount(uuid);
    model.setServiceEndpoint(m_service + "/message/2");

    model.setMemberDids(QStringList() << "did:plc:mqxsuw5b5rhpwo4lw6iwlid5");

    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait(10 * 1000);
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
}

void chat_test::test_ChatLogSubscriber()
{
    ChatLogSubscriber *log = ChatLogSubscriber::getInstance();
    ChatLogConnector connector(this);

    AtProtocolInterface::AccountData account;
    account.service = m_service + "/message/1";
    account.service_endpoint = m_service + "/message/1";
    account.did = "did:plc:ipj5qejfoqu6eukvt72uhyit";
    account.handle = "handle";
    account.accessJwt = "access_jwt";
    account.refreshJwt = "refresh_jwt";

    log->setAccount(account, &connector);
    // log->start(account, "2222222222aa6");
    {
        QSignalSpy spy(&connector,
                       SIGNAL(receiveLogs(const AtProtocolInterface::ChatBskyConvoGetLog &)));
        log->start(account, "2222222222aa6");
        spy.wait();
        spy.wait();
        spy.wait();
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        log->stop(account);
    }
}

QTEST_MAIN(chat_test)

#include "tst_chat_test.moc"
