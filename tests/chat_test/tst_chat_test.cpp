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
    QCOMPARE_NE(m_listenPort, 0);
}

void chat_test::cleanupTestCase() { }

void chat_test::test_ChatListModel()
{
    int i = 0;
    ChatListModel model;

    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/list", "id", "pass", "did:plc:ipj5qejfoqu6eukvt72uhyit",
            "handle", "email", "accessJwt", "refreshJwt", true);
    AccountManager::getInstance()->updateServiceEndpoint(uuid, m_service + "/list");
    model.setAccount(uuid);

    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait();
        QCOMPARE(spy.count(), 2);
    }

    QCOMPARE(model.rowCount(), 2);

    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait();
        QCOMPARE(spy.count(), 2);
    }

    QCOMPARE(model.rowCount(), 2);

    i = 0;
    QCOMPARE(model.item(i, ChatListModel::IdRole).toString(), "c2222");

    i = 1;
    QCOMPARE(model.item(i, ChatListModel::IdRole).toString(), "c1111");

    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getNext();
        spy.wait();
        QCOMPARE(spy.count(), 2);
    }

    QCOMPARE(model.rowCount(), 4);

    i = 0;
    QCOMPARE(model.item(i, ChatListModel::IdRole).toString(), "c2222");

    i = 1;
    QCOMPARE(model.item(i, ChatListModel::IdRole).toString(), "c1111");

    i = 2;
    QCOMPARE(model.item(i, ChatListModel::IdRole).toString(), "c4444");

    i = 3;
    QCOMPARE(model.item(i, ChatListModel::IdRole).toString(), "c3333");
}

void chat_test::test_ChatMessageListModel()
{
    ChatMessageListModel model;
    int i = 0;

    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/message/1", "id", "pass", "did:plc:ipj5qejfoqu6eukvt72uhyit",
            "handle", "email", "accessJwt", "refreshJwt", true);
    AccountManager::getInstance()->updateServiceEndpoint(uuid, m_service + "/message/1");
    model.setAccount(uuid);

    model.setConvoId("3ksrqt7eebs2b");
    model.setAutoLoading(false);

    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait(10 * 1000);
        QCOMPARE(spy.count(), 2);
    }

    QCOMPARE(model.rowCount(), 5);
    i = 0;
    QCOMPARE(model.item(i, ChatMessageListModel::IdRole).toString(), "3ksyzepxa2s2m");
    i = 1;
    QCOMPARE(model.item(i, ChatMessageListModel::IdRole).toString(), "3ksyzejyngk2s");
    i = 2;
    QCOMPARE(model.item(i, ChatMessageListModel::IdRole).toString(), "3ksyzdsn7hk2m");
    i = 3;
    QCOMPARE(model.item(i, ChatMessageListModel::IdRole).toString(), "3ksyzdmpzqs2s");
    i = 4;
    QCOMPARE(model.item(i, ChatMessageListModel::IdRole).toString(), "3ksyzdesy3s2m");

    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait();
        QCOMPARE(spy.count(), 2);
    }

    QCOMPARE(model.rowCount(), 5);
    i = 0;
    QCOMPARE(model.item(i, ChatMessageListModel::IdRole).toString(), "3ksyzepxa2s2m");
    i = 1;
    QCOMPARE(model.item(i, ChatMessageListModel::IdRole).toString(), "3ksyzejyngk2s");
    i = 2;
    QCOMPARE(model.item(i, ChatMessageListModel::IdRole).toString(), "3ksyzdsn7hk2m");
    i = 3;
    QCOMPARE(model.item(i, ChatMessageListModel::IdRole).toString(), "3ksyzdmpzqs2s");
    i = 4;
    QCOMPARE(model.item(i, ChatMessageListModel::IdRole).toString(), "3ksyzdesy3s2m");

    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getNext();
        spy.wait();
        QCOMPARE(spy.count(), 2);
    }

    QCOMPARE(model.rowCount(), 10);
    i = 0;
    QCOMPARE(model.item(i, ChatMessageListModel::IdRole).toString(), "3ksyzepxa2s2m");
    i = 1;
    QCOMPARE(model.item(i, ChatMessageListModel::IdRole).toString(), "3ksyzejyngk2s");
    i = 2;
    QCOMPARE(model.item(i, ChatMessageListModel::IdRole).toString(), "3ksyzdsn7hk2m");
    i = 3;
    QCOMPARE(model.item(i, ChatMessageListModel::IdRole).toString(), "3ksyzdmpzqs2s");
    i = 4;
    QCOMPARE(model.item(i, ChatMessageListModel::IdRole).toString(), "3ksyzdesy3s2m");
    i = 5;
    QCOMPARE(model.item(i, ChatMessageListModel::IdRole).toString(), "3ksyzd6vyak2s");
    i = 6;
    QCOMPARE(model.item(i, ChatMessageListModel::IdRole).toString(), "3ksyzcwz7es2s");
    i = 7;
    QCOMPARE(model.item(i, ChatMessageListModel::IdRole).toString(), "3ksyzbti4ns2k");
    i = 8;
    QCOMPARE(model.item(i, ChatMessageListModel::IdRole).toString(), "3ksyzbmczm22s");
    i = 9;
    QCOMPARE(model.item(i, ChatMessageListModel::IdRole).toString(), "3ksyzbg7zek2m");
}

void chat_test::test_ChatMessageListModelByMembers()
{
    ChatMessageListModel model;
    int i = 0;

    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/message/2", "id", "pass", "did:plc:ipj5qejfoqu6eukvt72uhyit",
            "handle", "email", "accessJwt", "refreshJwt", true);
    AccountManager::getInstance()->updateServiceEndpoint(uuid, m_service + "/message/2");
    model.setAccount(uuid);

    model.setMemberDids(QStringList() << "did:plc:mqxsuw5b5rhpwo4lw6iwlid5");

    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait(10 * 1000);
        QCOMPARE(spy.count(), 2);
    }

    QCOMPARE(model.rowCount(), 5);
    i = 0;
    QCOMPARE(model.item(i, ChatMessageListModel::IdRole).toString(), "3ksyzepxa2s2m");
    i = 1;
    QCOMPARE(model.item(i, ChatMessageListModel::IdRole).toString(), "3ksyzejyngk2s");
    i = 2;
    QCOMPARE(model.item(i, ChatMessageListModel::IdRole).toString(), "3ksyzdsn7hk2m");
    i = 3;
    QCOMPARE(model.item(i, ChatMessageListModel::IdRole).toString(), "3ksyzdmpzqs2s");
    i = 4;
    QCOMPARE(model.item(i, ChatMessageListModel::IdRole).toString(), "3ksyzdesy3s2m");
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
        QCOMPARE(spy.count(), 2);
        log->stop(account);
    }
}

QTEST_MAIN(chat_test)

#include "tst_chat_test.moc"
