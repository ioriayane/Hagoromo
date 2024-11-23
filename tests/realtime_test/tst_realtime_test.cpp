#include <QtTest>
#include <QCoreApplication>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonDocument>
#include <QWebSocketServer>

#include "webserver.h"
#include "realtime/abstractpostselector.h"
#include "realtime/firehosereceiver.h"
#include "realtime/realtimefeedlistmodel.h"
#include "realtime/editselectorlistmodel.h"
#include "tools/accountmanager.h"

using namespace RealtimeFeed;

class realtime_test : public QObject
{
    Q_OBJECT

public:
    realtime_test();
    ~realtime_test();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_PostSelector();
    void test_FirehoseReceiver();
    void test_Websock();
    void test_RealtimeFeedListModel();
    void test_EditSelectorListModel();
    void test_EditSelectorListModel_append();
    void test_EditSelectorListModel_save();

private:
    QList<UserInfo> extractFromArray(const QJsonArray &array) const;
    QJsonDocument loadJson(const QString &path);

    // QWebSocketServer m_server;
    // QList<QWebSocket *> m_clients;

    WebServer m_mockServer;
    quint16 m_listenPort;
    QString m_service;
};

realtime_test::realtime_test()
// : m_server(QStringLiteral("name"), QWebSocketServer::SecureMode, this)
{
    QCoreApplication::setOrganizationName(QStringLiteral("relog"));
    QCoreApplication::setApplicationName(QStringLiteral("Hagoromo_unittest"));

    m_listenPort = m_mockServer.listen(QHostAddress::LocalHost, 0);
    m_service = QString("http://localhost:%1/response").arg(m_listenPort);

    FirehoseReceiver::getInstance()->forUnittest = true;
}

realtime_test::~realtime_test() { }

void realtime_test::initTestCase()
{

    // if (m_server.listen(QHostAddress::LocalHost, 0)) {
    //     qDebug().noquote() << "Start WebSocket Server";
    //     connect(&m_server, &QWebSocketServer::newConnection, this, [=]() {
    //         QWebSocket *pSocket = m_server.nextPendingConnection();

    //         qDebug() << "QWebSocketServer::newConnection";
    //         qDebug() << "Client connected:" << pSocket->peerName() << pSocket->origin();

    //         connect(pSocket, &QWebSocket::binaryMessageReceived, this,
    //                 [=](const QByteArray &message) {
    //                     qDebug().noquote() << "QWebSocket::binaryMessageReceived";
    //                 });
    //         connect(pSocket, &QWebSocket::disconnected, this, [=]() {
    //             qDebug().noquote() << "QWebSocket::disconnected";
    //             QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    //             qDebug() << "socketDisconnected:" << pClient;
    //             if (pClient) {
    //                 m_clients.removeAll(pClient);
    //                 pClient->deleteLater();
    //             }
    //         });
    //         m_clients << pSocket;
    //     });
    //     connect(&m_server, &QWebSocketServer::sslErrors, this,
    //             [=](const QList<QSslError> &errors) { qDebug().noquote() << errors; });
    // }
}

void realtime_test::cleanupTestCase()
{
    // m_server.close();
    // qDeleteAll(m_clients.begin(), m_clients.end());
    // m_clients.clear();

    FirehoseReceiver *recv = FirehoseReceiver::getInstance();
    {
        QSignalSpy spy(recv, SIGNAL(disconnectFromService()));
        recv->removeAllSelector();
        spy.wait();
        // QVERIFY(spy.count() == 1);
    }
}

void realtime_test::test_PostSelector()
{
    QFile file(":/data/selector/selector.json");
    QVERIFY(file.open(QFile::ReadOnly));
    QJsonDocument json_doc = QJsonDocument::fromJson(file.readAll());
    QVERIFY(json_doc.isArray());
    for (const auto json_item : json_doc.array()) {
        QVERIFY(json_item.isObject());
        QString description = json_item.toObject().value("description").toString();
        qDebug().noquote() << description;
        QVERIFY(json_item.toObject().contains("following"));
        QVERIFY(json_item.toObject().contains("followers"));
        QVERIFY(json_item.toObject().contains("selector"));
        QVERIFY(json_item.toObject().contains("data"));
        QList<UserInfo> following =
                extractFromArray(json_item.toObject().value("following").toArray());
        QList<UserInfo> followers =
                extractFromArray(json_item.toObject().value("followers").toArray());
        QVERIFY(following.isEmpty() == false);
        QVERIFY(followers.isEmpty() == false);

        AbstractPostSelector *root = AbstractPostSelector::create(
                json_item.toObject().value("selector").toObject(), this);
        QVERIFY(root != nullptr);
        root->setFollowing(following);
        root->setFollowers(followers);
        root->setDid("did:plc:me");
        root->setHandle("handle.example.com");
        root->setDisplayName("my display name");
        root->setReady(true);

        QVERIFY(root->needFollowing()
                == json_item.toObject().value("except").toObject().value("needFollowing").toBool());
        QVERIFY(root->needFollowers()
                == json_item.toObject().value("except").toObject().value("needFollowers").toBool());

        bool equal = (QJsonDocument::fromJson(root->toString().toUtf8()).object()
                      == json_item.toObject().value("selector").toObject());
        if (!equal) {
            qDebug().noquote().nospace() << root->toString();
        }
        QVERIFY(equal);

        int i = 0;
        for (const auto data : json_item.toObject().value("data").toArray()) {
            QVERIFY2(root->judge(data.toObject().value("payload").toObject())
                             == data.toObject().value("except").toBool(),
                     QString("%1(%2)").arg(description).arg(i++).toLocal8Bit());

            qDebug().noquote() << AbstractPostSelector::getOperationUris(
                    data.toObject().value("payload").toObject());
        }

        delete root;
    }
}

void realtime_test::test_FirehoseReceiver()
{
    FirehoseReceiver *recv = FirehoseReceiver::getInstance();

    QObject parent1;
    QObject parent2;

    recv->appendSelector(AbstractPostSelector::create(
            QJsonDocument::fromJson("{\"me\":{}}").object(), &parent1));
    recv->appendSelector(AbstractPostSelector::create(
            QJsonDocument::fromJson("{\"followers\":{}}").object(), &parent2));

    QVERIFY(recv->countSelector() == 2);

    const AbstractPostSelector *s1 = recv->getSelector(&parent1);
    const AbstractPostSelector *s2 = recv->getSelector(&parent2);

    QVERIFY(s1 != nullptr);
    QVERIFY(s2 != nullptr);

    QVERIFY(s1->needFollowers() == false);
    QVERIFY(s1->needFollowing() == false);
    QVERIFY(s2->needFollowers() == true);
    QVERIFY(s2->needFollowing() == false);

    recv->removeSelector(&parent1);
    QVERIFY(recv->countSelector() == 1);

    QVERIFY(recv->getSelector(&parent1) == nullptr);
    QVERIFY(recv->getSelector(&parent2) != nullptr);

    QVERIFY(recv->containsSelector(&parent1) == false);
    QVERIFY(recv->containsSelector(&parent2) == true);

    // recv->removeAllSelector();
}

void realtime_test::test_Websock()
{

#if 0
    FirehoseReceiver *recv = FirehoseReceiver::getInstance();
    QObject parent1;
    recv->appendSelector(AbstractPostSelector::create(
            QJsonDocument::fromJson("{\"not\":{\"me\":{}}}").object(), &parent1));
    // recv->setServiceEndpoint(m_server.serverUrl().toString());
    recv->getSelector(&parent1)->setDid("did:plc:mqxsuw5b5rhpwo4lw6iwlid5");
    recv->getSelector(&parent1)->setReady(true);

    {
        QSignalSpy spy(recv, SIGNAL(connectedToService()));
        recv->start();
        spy.wait();
        QVERIFY(spy.count() == 1);
    }
    {
        QSignalSpy spy(recv->getSelector(&parent1), SIGNAL(selected(const QJsonObject &)));
        spy.wait(60 * 1000);
        qDebug().noquote() << "selected : spy.count" << spy.count();
        QVERIFY2(spy.count() >= 1, QString::number(spy.count()).toLocal8Bit());
    }
    {
        QSignalSpy spy(recv, SIGNAL(disconnectFromService()));
        qDebug().noquote() << "before removeSelector";
        recv->removeSelector(&parent1);
        spy.wait();
        QVERIFY(spy.count() == 1);
    }
#endif
}

void realtime_test::test_RealtimeFeedListModel()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/realtime/1", "id", "pass", "did:plc:mqxsuw5b5rhpwo4lw6iwlid5",
            "handle", "email", "accessJwt", "refreshJwt", true);

    RealtimeFeedListModel model;
    model.setAccount(uuid);

    // model.setSelectorJson("{\"not\":{\"me\":{}}}");
    // model.setSelectorJson("{\"not\":{\"following\":{}}}");
    // model.setSelectorJson("{\"not\":{\"followers\":{}}}");
    model.setSelectorJson("{\"or\": [{\"following\": {}},{\"followers\": {}},{\"list\": "
                          "{\"uri\":\"at://did:plc:ipj5qejfoqu6eukvt72uhyit/app.bsky.graph.list/"
                          "3kcj52ovctd2h\",\"name\":\"My Accounts\"}}]}");
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait(20 * 1000);
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }

    qDebug().noquote() << "---------------------------";
    model.getLatest();

    FirehoseReceiver *recv = FirehoseReceiver::getInstance();
    AbstractPostSelector *s = recv->getSelector(&model);
    QVERIFY(s != nullptr);

    {
        QSignalSpy spy(recv, SIGNAL(connectedToService()));
        recv->start();
        spy.wait();
        QVERIFY(spy.count() == 1);
    }

    QVERIFY(s->did() == "did:plc:mqxsuw5b5rhpwo4lw6iwlid5");
    QVERIFY(s->ready() == true);
    QVERIFY(s->needFollowing() == true);
    QVERIFY(s->needFollowers() == true);

    QJsonDocument json_doc;

    qDebug().noquote() << "---------------------------";
    json_doc = loadJson(":/data/realtimemodel/recv_data_1.json");
    QVERIFY(json_doc.isObject());
    QVERIFY2(model.rowCount() == 0, QString::number(model.rowCount()).toLocal8Bit());
    {
        QSignalSpy spy(&model, SIGNAL(rowsInserted(const QModelIndex &, int, int)));
        recv->testReceived(json_doc.object());
        spy.wait();
        QVERIFY(spy.count() == 1);
    }
    QVERIFY2(model.rowCount() == 1, QString::number(model.rowCount()).toLocal8Bit());

    QVERIFY(model.item(0, TimelineListModel::CidRole).toString()
            == "bafyreigoon4vpg3axqlvrzyxcpmwh4ihra4hbqd5uh3e774bbjjnla5ajq");
    QVERIFY(model.item(0, TimelineListModel::RecordTextPlainRole).toString() == "reply3");

    qDebug().noquote() << "---------------------------";
    uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/realtime/2", "id", "pass", "did:plc:mqxsuw5b5rhpwo4lw6iwlid5",
            "handle", "email", "accessJwt", "refreshJwt", true);
    model.setAccount(uuid);
    json_doc = loadJson(":/data/realtimemodel/recv_data_2.json");
    QVERIFY(json_doc.isObject());
    {
        QSignalSpy spy(&model, SIGNAL(rowsInserted(const QModelIndex &, int, int)));
        recv->testReceived(json_doc.object());
        spy.wait();
        QVERIFY(spy.count() == 1);
    }
    QVERIFY2(model.rowCount() == 2, QString::number(model.rowCount()).toLocal8Bit());

    QVERIFY(model.item(0, TimelineListModel::CidRole).toString()
            == "bafyreiadfsi4feaygror6ekk5j7mlhquau7zbf3jlxasmjjcsdjs75zbjq");
    QVERIFY(model.item(0, TimelineListModel::RecordTextPlainRole).toString() == "update hagoromo");
    QVERIFY(model.item(0, TimelineListModel::IsRepostedByRole).toBool() == true);
    QVERIFY(model.item(0, TimelineListModel::RepostedByDisplayNameRole).toString() == "");
    QVERIFY(model.item(0, TimelineListModel::RepostedByHandleRole).toString()
            == "ioriayane.bsky.social");

    QVERIFY(model.item(1, TimelineListModel::CidRole).toString()
            == "bafyreigoon4vpg3axqlvrzyxcpmwh4ihra4hbqd5uh3e774bbjjnla5ajq");
    QVERIFY(model.item(1, TimelineListModel::RecordTextPlainRole).toString() == "reply3");
    QVERIFY(model.item(1, TimelineListModel::IsRepostedByRole).toBool() == false);

    qDebug().noquote() << "---------------------------";
    uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/realtime/3", "id", "pass", "did:plc:mqxsuw5b5rhpwo4lw6iwlid5",
            "handle", "email", "accessJwt", "refreshJwt", true);
    model.setAccount(uuid);
    json_doc = loadJson(":/data/realtimemodel/recv_data_3.json");
    recv->testReceived(json_doc.object());
    QVERIFY2(model.rowCount() == 2, QString::number(model.rowCount()).toLocal8Bit());

    qDebug().noquote() << "---------------------------";
    json_doc = loadJson(":/data/realtimemodel/recv_data_4.json");
    recv->testReceived(json_doc.object());
    QVERIFY2(model.rowCount() == 2, QString::number(model.rowCount()).toLocal8Bit());

    qDebug().noquote() << "---------------------------";
    json_doc = loadJson(":/data/realtimemodel/recv_data_3.json");
    QVERIFY(json_doc.isObject());
    {
        QSignalSpy spy(&model, SIGNAL(rowsInserted(const QModelIndex &, int, int)));
        recv->testReceived(json_doc.object());
        spy.wait();
        QVERIFY(spy.count() == 1);
    }
    QVERIFY2(model.rowCount() == 3, QString::number(model.rowCount()).toLocal8Bit());
    QVERIFY(model.item(0, TimelineListModel::CidRole).toString()
            == "bafyreigoon4vpg3axqlvrzyxcpmwh4ihra4hbqd5uh3e774bbjjnla5ajq3");
    QVERIFY(model.item(0, TimelineListModel::RecordTextPlainRole).toString() == "post 3");
    QVERIFY(model.item(0, TimelineListModel::IsRepostedByRole).toBool() == false);

    qDebug().noquote() << "---------------------------";
    uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/realtime/4", "id", "pass", "did:plc:mqxsuw5b5rhpwo4lw6iwlid5",
            "handle", "email", "accessJwt", "refreshJwt", true);
    model.setAccount(uuid);
    json_doc = loadJson(":/data/realtimemodel/recv_data_5.json");
    QVERIFY(json_doc.isObject());
    {
        QSignalSpy spy(&model, SIGNAL(rowsInserted(const QModelIndex &, int, int)));
        recv->testReceived(json_doc.object());
        spy.wait();
        QVERIFY(spy.count() == 1);
    }
    QVERIFY2(model.rowCount() == 4, QString::number(model.rowCount()).toLocal8Bit());
    QVERIFY(model.item(0, TimelineListModel::CidRole).toString()
            == "bafyreigoon4vpg3axqlvrzyxcpmwh4ihra4hbqd5uh3e774bbjjnla5ajq5");
    QVERIFY(model.item(0, TimelineListModel::RecordTextPlainRole).toString() == "post 5");
    QVERIFY(model.item(0, TimelineListModel::IsRepostedByRole).toBool() == false);
}

void realtime_test::test_EditSelectorListModel()
{
    EditSelectorListModel model;

    QVERIFY2(model.rowCount() == 0, QString::number(model.rowCount()).toLocal8Bit());
    QVERIFY(model.item(0, EditSelectorListModel::EditSelectorListModelRoles::TypeRole)
            == QVariant());
    QVERIFY(model.valid() == false);

    //----
    model.setSelectorJson("{\"me\":{}}");
    QVERIFY2(model.rowCount() == 1, QString::number(model.rowCount()).toLocal8Bit());
    QVERIFY(model.valid() == true);
    QVERIFY2(model.item(0, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString()
                     == "me",
             model.item(0, EditSelectorListModel::EditSelectorListModelRoles::TypeRole)
                     .toString()
                     .toLocal8Bit());
    QVERIFY(model.item(1, EditSelectorListModel::EditSelectorListModelRoles::TypeRole)
            == QVariant());

    QVERIFY2(model.item(0, EditSelectorListModel::EditSelectorListModelRoles::IndentRole).toInt()
                     == 0,
             QString::number(
                     model.item(0, EditSelectorListModel::EditSelectorListModelRoles::IndentRole)
                             .toInt())
                     .toLocal8Bit());

    //----
    model.setSelectorJson("{\"not\":{\"me\":{}}}");
    QVERIFY2(model.rowCount() == 2, QString::number(model.rowCount()).toLocal8Bit());
    QVERIFY(model.valid() == true);
    QVERIFY2(model.item(0, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString()
                     == "not",
             model.item(0, EditSelectorListModel::EditSelectorListModelRoles::TypeRole)
                     .toString()
                     .toLocal8Bit());
    QVERIFY2(model.item(1, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString()
                     == "me",
             model.item(1, EditSelectorListModel::EditSelectorListModelRoles::TypeRole)
                     .toString()
                     .toLocal8Bit());
    QVERIFY(model.item(2, EditSelectorListModel::EditSelectorListModelRoles::TypeRole)
            == QVariant());

    QVERIFY2(model.item(0, EditSelectorListModel::EditSelectorListModelRoles::IndentRole).toInt()
                     == 0,
             QString::number(
                     model.item(0, EditSelectorListModel::EditSelectorListModelRoles::IndentRole)
                             .toInt())
                     .toLocal8Bit());
    QVERIFY2(model.item(1, EditSelectorListModel::EditSelectorListModelRoles::IndentRole).toInt()
                     == 1,
             QString::number(
                     model.item(1, EditSelectorListModel::EditSelectorListModelRoles::IndentRole)
                             .toInt())
                     .toLocal8Bit());

    //----
    model.setSelectorJson("{\"or\": [{\"following\": {}},{\"followers\": {}}]}");
    QVERIFY2(model.rowCount() == 3, QString::number(model.rowCount()).toLocal8Bit());
    QVERIFY(model.valid() == true);
    QVERIFY2(model.item(0, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString()
                     == "or",
             model.item(0, EditSelectorListModel::EditSelectorListModelRoles::TypeRole)
                     .toString()
                     .toLocal8Bit());
    QVERIFY2(model.item(1, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString()
                     == "following",
             model.item(1, EditSelectorListModel::EditSelectorListModelRoles::TypeRole)
                     .toString()
                     .toLocal8Bit());
    QVERIFY2(model.item(2, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString()
                     == "followers",
             model.item(2, EditSelectorListModel::EditSelectorListModelRoles::TypeRole)
                     .toString()
                     .toLocal8Bit());
    QVERIFY(model.item(3, EditSelectorListModel::EditSelectorListModelRoles::TypeRole)
            == QVariant());

    QVERIFY2(model.item(0, EditSelectorListModel::EditSelectorListModelRoles::IndentRole).toInt()
                     == 0,
             QString::number(
                     model.item(0, EditSelectorListModel::EditSelectorListModelRoles::IndentRole)
                             .toInt())
                     .toLocal8Bit());
    QVERIFY2(model.item(1, EditSelectorListModel::EditSelectorListModelRoles::IndentRole).toInt()
                     == 1,
             QString::number(
                     model.item(1, EditSelectorListModel::EditSelectorListModelRoles::IndentRole)
                             .toInt())
                     .toLocal8Bit());
    QVERIFY2(model.item(2, EditSelectorListModel::EditSelectorListModelRoles::IndentRole).toInt()
                     == 1,
             QString::number(
                     model.item(1, EditSelectorListModel::EditSelectorListModelRoles::IndentRole)
                             .toInt())
                     .toLocal8Bit());

    //----
    model.setSelectorJson("{\"and\": [{\"followers\": {}},{\"following\": {}},{\"me\":{}}]}");
    QVERIFY2(model.rowCount() == 4, QString::number(model.rowCount()).toLocal8Bit());
    QVERIFY(model.valid() == true);
    QVERIFY2(model.item(0, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString()
                     == "and",
             model.item(0, EditSelectorListModel::EditSelectorListModelRoles::TypeRole)
                     .toString()
                     .toLocal8Bit());
    QVERIFY2(model.item(1, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString()
                     == "followers",
             model.item(1, EditSelectorListModel::EditSelectorListModelRoles::TypeRole)
                     .toString()
                     .toLocal8Bit());
    QVERIFY2(model.item(2, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString()
                     == "following",
             model.item(2, EditSelectorListModel::EditSelectorListModelRoles::TypeRole)
                     .toString()
                     .toLocal8Bit());
    QVERIFY2(model.item(3, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString()
                     == "me",
             model.item(3, EditSelectorListModel::EditSelectorListModelRoles::TypeRole)
                     .toString()
                     .toLocal8Bit());
    QVERIFY(model.item(4, EditSelectorListModel::EditSelectorListModelRoles::TypeRole)
            == QVariant());

    QVERIFY2(model.item(0, EditSelectorListModel::EditSelectorListModelRoles::IndentRole).toInt()
                     == 0,
             QString::number(
                     model.item(0, EditSelectorListModel::EditSelectorListModelRoles::IndentRole)
                             .toInt())
                     .toLocal8Bit());
    QVERIFY2(model.item(1, EditSelectorListModel::EditSelectorListModelRoles::IndentRole).toInt()
                     == 1,
             QString::number(
                     model.item(1, EditSelectorListModel::EditSelectorListModelRoles::IndentRole)
                             .toInt())
                     .toLocal8Bit());
    QVERIFY2(model.item(2, EditSelectorListModel::EditSelectorListModelRoles::IndentRole).toInt()
                     == 1,
             QString::number(
                     model.item(2, EditSelectorListModel::EditSelectorListModelRoles::IndentRole)
                             .toInt())
                     .toLocal8Bit());
    QVERIFY2(model.item(3, EditSelectorListModel::EditSelectorListModelRoles::IndentRole).toInt()
                     == 1,
             QString::number(
                     model.item(3, EditSelectorListModel::EditSelectorListModelRoles::IndentRole)
                             .toInt())
                     .toLocal8Bit());

    //----
    model.setSelectorJson("{\"and\": [{\"not\": {\"following\": {}}},{\"xor\": [{\"following\": "
                          "{}},{\"not\": {\"followers\": {}}}]},{\"following\": {}}]}");
    QVERIFY2(model.rowCount() == 8, QString::number(model.rowCount()).toLocal8Bit());
    QVERIFY(model.valid() == true);
    QVERIFY2(model.item(0, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString()
                     == "and",
             model.item(0, EditSelectorListModel::EditSelectorListModelRoles::TypeRole)
                     .toString()
                     .toLocal8Bit());
    QVERIFY2(model.item(1, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString()
                     == "not",
             model.item(1, EditSelectorListModel::EditSelectorListModelRoles::TypeRole)
                     .toString()
                     .toLocal8Bit());
    QVERIFY2(model.item(2, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString()
                     == "following",
             model.item(2, EditSelectorListModel::EditSelectorListModelRoles::TypeRole)
                     .toString()
                     .toLocal8Bit());
    QVERIFY2(model.item(3, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString()
                     == "xor",
             model.item(3, EditSelectorListModel::EditSelectorListModelRoles::TypeRole)
                     .toString()
                     .toLocal8Bit());
    QVERIFY2(model.item(4, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString()
                     == "following",
             model.item(4, EditSelectorListModel::EditSelectorListModelRoles::TypeRole)
                     .toString()
                     .toLocal8Bit());
    QVERIFY2(model.item(5, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString()
                     == "not",
             model.item(5, EditSelectorListModel::EditSelectorListModelRoles::TypeRole)
                     .toString()
                     .toLocal8Bit());
    QVERIFY2(model.item(6, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString()
                     == "followers",
             model.item(6, EditSelectorListModel::EditSelectorListModelRoles::TypeRole)
                     .toString()
                     .toLocal8Bit());
    QVERIFY2(model.item(7, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString()
                     == "following",
             model.item(7, EditSelectorListModel::EditSelectorListModelRoles::TypeRole)
                     .toString()
                     .toLocal8Bit());
    QVERIFY(model.item(8, EditSelectorListModel::EditSelectorListModelRoles::TypeRole)
            == QVariant());

    QVERIFY2(model.item(0, EditSelectorListModel::EditSelectorListModelRoles::IndentRole).toInt()
                     == 0,
             QString::number(
                     model.item(0, EditSelectorListModel::EditSelectorListModelRoles::IndentRole)
                             .toInt())
                     .toLocal8Bit());
    QVERIFY2(model.item(1, EditSelectorListModel::EditSelectorListModelRoles::IndentRole).toInt()
                     == 1,
             QString::number(
                     model.item(1, EditSelectorListModel::EditSelectorListModelRoles::IndentRole)
                             .toInt())
                     .toLocal8Bit());
    QVERIFY2(model.item(2, EditSelectorListModel::EditSelectorListModelRoles::IndentRole).toInt()
                     == 2,
             QString::number(
                     model.item(2, EditSelectorListModel::EditSelectorListModelRoles::IndentRole)
                             .toInt())
                     .toLocal8Bit());
    QVERIFY2(model.item(3, EditSelectorListModel::EditSelectorListModelRoles::IndentRole).toInt()
                     == 1,
             QString::number(
                     model.item(3, EditSelectorListModel::EditSelectorListModelRoles::IndentRole)
                             .toInt())
                     .toLocal8Bit());
    QVERIFY2(model.item(4, EditSelectorListModel::EditSelectorListModelRoles::IndentRole).toInt()
                     == 2,
             QString::number(
                     model.item(4, EditSelectorListModel::EditSelectorListModelRoles::IndentRole)
                             .toInt())
                     .toLocal8Bit());
    QVERIFY2(model.item(5, EditSelectorListModel::EditSelectorListModelRoles::IndentRole).toInt()
                     == 2,
             QString::number(
                     model.item(5, EditSelectorListModel::EditSelectorListModelRoles::IndentRole)
                             .toInt())
                     .toLocal8Bit());
    QVERIFY2(model.item(6, EditSelectorListModel::EditSelectorListModelRoles::IndentRole).toInt()
                     == 3,
             QString::number(
                     model.item(6, EditSelectorListModel::EditSelectorListModelRoles::IndentRole)
                             .toInt())
                     .toLocal8Bit());
    QVERIFY2(model.item(7, EditSelectorListModel::EditSelectorListModelRoles::IndentRole).toInt()
                     == 1,
             QString::number(
                     model.item(7, EditSelectorListModel::EditSelectorListModelRoles::IndentRole)
                             .toInt())
                     .toLocal8Bit());

    QVERIFY2(model.item(0, EditSelectorListModel::EditSelectorListModelRoles::DisplayTypeRole)
                             .toString()
                     == "AND",
             model.item(0, EditSelectorListModel::EditSelectorListModelRoles::DisplayTypeRole)
                     .toString()
                     .toLocal8Bit());
    QVERIFY2(model.item(6, EditSelectorListModel::EditSelectorListModelRoles::DisplayTypeRole)
                             .toString()
                     == "Followers",
             model.item(6, EditSelectorListModel::EditSelectorListModelRoles::DisplayTypeRole)
                     .toString()
                     .toLocal8Bit());

    QVERIFY2(model.item(0, EditSelectorListModel::EditSelectorListModelRoles::CanHaveRole)
                             .toStringList()
                     == QStringList() << "following"
                                      << "followers"
                                      << "me"
                                      << "and"
                                      << "or"
                                      << "not"
                                      << "xor",
             model.item(0, EditSelectorListModel::EditSelectorListModelRoles::CanHaveRole)
                     .toString()
                     .toLocal8Bit());
    QVERIFY2(model.item(6, EditSelectorListModel::EditSelectorListModelRoles::CanHaveRole)
                             .toStringList()
                     == QStringList(),
             model.item(6, EditSelectorListModel::EditSelectorListModelRoles::CanHaveRole)
                     .toString()
                     .toLocal8Bit());
}

void realtime_test::test_EditSelectorListModel_append()
{
    EditSelectorListModel model;

    model.appendChild(-1, "me");
    QVERIFY2(model.rowCount() == 1, QString::number(model.rowCount()).toLocal8Bit());
    QVERIFY(model.valid() == true);
    QVERIFY2(model.item(0, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString()
                     == "me",
             model.item(0, EditSelectorListModel::EditSelectorListModelRoles::TypeRole)
                     .toString()
                     .toLocal8Bit());

    // don't append
    model.appendChild(0, "not");
    QVERIFY2(model.rowCount() == 1, QString::number(model.rowCount()).toLocal8Bit());
    QVERIFY(model.valid() == true);

    model.clear();

    qDebug() << "----";

    model.appendChild(-1, "and");
    QVERIFY2(model.rowCount() == 1, QString::number(model.rowCount()).toLocal8Bit());
    QVERIFY(model.valid() == false);
    QVERIFY2(model.item(0, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString()
                     == "and",
             model.item(0, EditSelectorListModel::EditSelectorListModelRoles::TypeRole)
                     .toString()
                     .toLocal8Bit());

    model.appendChild(0, "following");
    QVERIFY2(model.rowCount() == 2, QString::number(model.rowCount()).toLocal8Bit());
    QVERIFY(model.valid() == true);
    QVERIFY2(model.item(1, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString()
                     == "following",
             model.item(1, EditSelectorListModel::EditSelectorListModelRoles::TypeRole)
                     .toString()
                     .toLocal8Bit());

    model.appendChild(0, "or");
    QVERIFY2(model.rowCount() == 3, QString::number(model.rowCount()).toLocal8Bit());
    QVERIFY(model.valid() == false);
    QVERIFY2(model.item(2, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString()
                     == "or",
             model.item(2, EditSelectorListModel::EditSelectorListModelRoles::TypeRole)
                     .toString()
                     .toLocal8Bit());

    model.appendChild(2, "followers");
    QVERIFY2(model.rowCount() == 4, QString::number(model.rowCount()).toLocal8Bit());
    QVERIFY(model.valid() == true);
    QVERIFY2(model.item(3, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString()
                     == "followers",
             model.item(3, EditSelectorListModel::EditSelectorListModelRoles::TypeRole)
                     .toString()
                     .toLocal8Bit());

    model.appendChild(0, "xor");
    QVERIFY2(model.rowCount() == 5, QString::number(model.rowCount()).toLocal8Bit());
    QVERIFY(model.valid() == false);
    QVERIFY2(model.item(4, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString()
                     == "xor",
             model.item(4, EditSelectorListModel::EditSelectorListModelRoles::TypeRole)
                     .toString()
                     .toLocal8Bit());

    model.appendChild(2, "following");
    QVERIFY2(model.rowCount() == 6, QString::number(model.rowCount()).toLocal8Bit());
    QVERIFY(model.valid() == false);
    QVERIFY2(model.item(4, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString()
                     == "following",
             model.item(4, EditSelectorListModel::EditSelectorListModelRoles::TypeRole)
                     .toString()
                     .toLocal8Bit());
    QVERIFY2(model.item(5, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString()
                     == "xor",
             model.item(5, EditSelectorListModel::EditSelectorListModelRoles::TypeRole)
                     .toString()
                     .toLocal8Bit());

    model.remove(2);
    QVERIFY2(model.rowCount() == 3, QString::number(model.rowCount()).toLocal8Bit());
    QVERIFY(model.valid() == false);

    model.appendChild(2, "following");
    QVERIFY2(model.rowCount() == 4, QString::number(model.rowCount()).toLocal8Bit());
    QVERIFY(model.valid() == true);
    QVERIFY2(model.item(3, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString()
                     == "following",
             model.item(3, EditSelectorListModel::EditSelectorListModelRoles::TypeRole)
                     .toString()
                     .toLocal8Bit());

    // qDebug().noquote() << model.toJson();

    model.remove(0);
    QVERIFY2(model.rowCount() == 0, QString::number(model.rowCount()).toLocal8Bit());
    QVERIFY(model.valid() == false);
}

void realtime_test::test_EditSelectorListModel_save()
{

    EditSelectorListModel model;

    model.appendChild(-1, "me");
    QVERIFY(model.count() == 1);
    model.update(0, EditSelectorListModel::HasImageRole, true);
    QVERIFY(model.item(0, EditSelectorListModel::HasImageRole).toBool() == true);
    model.update(0, EditSelectorListModel::ImageCountRole, 3);
    QVERIFY(model.item(0, EditSelectorListModel::ImageCountRole).toInt() == 3);
    QVERIFY(model.toJson() == "{\"me\":{\"image\":{\"has\":true,\"count\":3}}}");
    model.update(0, EditSelectorListModel::HasImageRole, false);
    QVERIFY(model.item(0, EditSelectorListModel::HasImageRole).toBool() == false);
    QVERIFY(model.toJson() == "{\"me\":{}}");

    model.update(0, EditSelectorListModel::HasMovieRole, true);
    QVERIFY(model.item(0, EditSelectorListModel::HasMovieRole).toBool() == true);
    QVERIFY(model.toJson() == "{\"me\":{\"movie\":{\"has\":true}}}");
    model.update(0, EditSelectorListModel::HasMovieRole, false);
    QVERIFY(model.item(0, EditSelectorListModel::HasMovieRole).toBool() == false);
    QVERIFY(model.toJson() == "{\"me\":{}}");

    qDebug().noquote() << model.toJson();

    model.update(0, EditSelectorListModel::HasQuoteRole, true);
    QVERIFY(model.item(0, EditSelectorListModel::HasQuoteRole).toBool() == true);
    QVERIFY(model.toJson() == "{\"me\":{\"quote\":{\"has\":true}}}");
    model.update(0, EditSelectorListModel::HasQuoteRole, false);
    QVERIFY(model.item(0, EditSelectorListModel::HasQuoteRole).toBool() == false);

    model.update(0, EditSelectorListModel::HasImageRole, true);
    model.update(0, EditSelectorListModel::HasMovieRole, true);
    model.update(0, EditSelectorListModel::HasQuoteRole, true);
    QVERIFY(model.item(0, EditSelectorListModel::HasImageRole).toBool() == true);
    QVERIFY(model.item(0, EditSelectorListModel::HasMovieRole).toBool() == true);
    QVERIFY(model.item(0, EditSelectorListModel::HasQuoteRole).toBool() == true);
    QVERIFY(model.toJson()
            == "{\"me\":{\"image\":{\"has\":true,\"count\":3},\"movie\":{\"has\":true},\"quote\":{"
               "\"has\":true}}}");

    qDebug().noquote() << model.toJson();
    qDebug() << model.toJson();

    model.clear();
    model.appendChild(-1, "list");
    QVERIFY(model.count() == 1);

    QString uri = "at://did:plc:ipj5qejfoqu6eukvt72uhyit/app.bsky.graph.list/3kgybwfpfag2i";
    QString name = "illust";
    model.update(0, EditSelectorListModel::ListUriRole, uri);
    model.update(0, EditSelectorListModel::ListNameRole, name);
    QVERIFY(model.item(0, EditSelectorListModel::ListUriRole).toString() == uri);
    QVERIFY(model.item(0, EditSelectorListModel::ListNameRole).toString() == name);
    QVERIFY(model.toJson()
            == QString("{\"list\":{\"uri\":\"%1\",\"name\":\"%2\"}}").arg(uri, name));

    QString json;

    json = "{\"or\":[{\"and\":[{\"following\":{\"image\":{\"has\":true,\"count\":4}}"
           "},{\"followers\":{\"movie\":{\"has\":true}}}]},{\"me\":{}},{\"not\":{"
           "\"following\":{\"quote\":{\"has\":true}}}}]}";
    model.clear();
    model.setSelectorJson(json);
    QVERIFY(model.toJson() == json);

    json = "{\"and\":[{\"following\":{}},{\"list\":{\"uri\":\"at://"
           "did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.graph.list/"
           "3kflf2r3lwg2x\",\"name\":\"ice\"}}]}";
    model.clear();
    model.setSelectorJson(json);
    QVERIFY(model.toJson() == json);

    json = "{\"or\":[{\"and\":[{\"following\":{\"image\":{\"has\":true,\"count\":3}}},{\"list\":{"
           "\"uri\":\"at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.graph.list/"
           "3kflf2r3lwg2x\",\"name\":\"ice\"}}]},{\"me\":{}},{\"and\":[{\"followers\":{}},{"
           "\"list\":{\"uri\":\"at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.graph.list/"
           "3kflbnc4c4o2x\",\"name\":\"list\"}}]}]}";

    QJsonObject json_obj = QJsonDocument::fromJson(json.toUtf8()).object();
    QVERIFY(json_obj.isEmpty() == false);
    AbstractPostSelector *selector = AbstractPostSelector::create(json_obj, this);
    QVERIFY(selector != nullptr);
    QVERIFY(selector->toString() == json);
    QVERIFY(selector->getListUris()
            == QStringList()
                    << "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.graph.list/3kflf2r3lwg2x"
                    << "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.graph.list/3kflbnc4c4o2x");
    selector->deleteLater();
}

QList<UserInfo> realtime_test::extractFromArray(const QJsonArray &array) const
{
    QList<UserInfo> ret;
    for (const auto item : array) {
        UserInfo user;
        user.did = item.toObject().value("did").toString();
        user.rkey = item.toObject().value("rkey").toString();
        ret.append(user);
    }
    return ret;
}

QJsonDocument realtime_test::loadJson(const QString &path)
{
    QFile file(path);
    if (file.open(QFile::ReadOnly)) {
        return QJsonDocument::fromJson(file.readAll());
    } else {
        return QJsonDocument();
    }
}

QTEST_MAIN(realtime_test)

#include "tst_realtime_test.moc"
