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
#include "atprotocol/lexicons_func_unknown.h"

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
    void test_StructCopy();
    void test_PostSelector();
    void test_FirehoseReceiver();
    void test_Websock();
    void test_RealtimeFeedListModel();

private:
    QList<UserInfo> extractFromArray(const QJsonArray &array) const;

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
    QCoreApplication::setApplicationName(QStringLiteral("Hagoromo"));

    m_listenPort = m_mockServer.listen(QHostAddress::LocalHost, 0);
    m_service = QString("http://localhost:%1/response").arg(m_listenPort);
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

void realtime_test::test_StructCopy()
{
    {
        AtProtocolType::ComAtprotoSyncSubscribeRepos::BlockHeader header;
        AtProtocolType::ComAtprotoSyncSubscribeRepos::copyBlockHeader(
                QCborValue::fromJsonValue(
                        QJsonDocument::fromJson("{\"op\": 1,\"t\": \"#commit\"}").object()),
                header);
        QVERIFY2(header.op == 1, QString::number(header.op).toLocal8Bit());
        QVERIFY2(header.t == "#commit", header.t.toLocal8Bit());
    }

    {
        AtProtocolType::ComAtprotoSyncSubscribeRepos::BlockHeader header;
        AtProtocolType::ComAtprotoSyncSubscribeRepos::copyBlockHeader(
                QCborValue::fromJsonValue(
                        QJsonDocument::fromJson("{\"op\": 1,\"t\": \"#identity\"}").object()),
                header);
        QVERIFY2(header.op == 1, QString::number(header.op).toLocal8Bit());
        QVERIFY2(header.t == "#identity", header.t.toLocal8Bit());
    }

    {
        AtProtocolType::ComAtprotoSyncSubscribeRepos::BlockHeader header;
        AtProtocolType::ComAtprotoSyncSubscribeRepos::copyBlockHeader(
                QCborValue::fromJsonValue(QJsonDocument::fromJson("{\"op\": -1}").object()),
                header);
        QVERIFY2(header.op == -1, QString::number(header.op).toLocal8Bit());
        QVERIFY2(header.t == "", header.t.toLocal8Bit());
    }

    {
        AtProtocolType::ComAtprotoSyncSubscribeRepos::Commit commit;
        AtProtocolType::ComAtprotoSyncSubscribeRepos::copyCommit(
                QCborValue::fromJsonValue(
                        QJsonDocument::fromJson(
                                "{\"blobs\": [],\"blocks\": [],\"commit\": {\"$link\": "
                                "\"bafyreibj5esksh55hx3gjdaqcsfmdhbc6kcghhlkj7ikquoddnuwzueqde\"},"
                                "\"ops\": [{\"action\": \"create\",\"cid\": {\"$link\": "
                                "\"bafyreihj7nokeio7tw4krnuqmuj6htykx7i4fscgcajbvkrmbuqvh22pee\"},"
                                "\"path\": \"app.bsky.feed.post/3ktudvq2ovc2k\"}],\"prev\": "
                                "null,\"rebase\": false,\"repo\": \"did:plc:user1\",\"rev\": "
                                "\"3ktudvq2uqs2k\",\"seq\": 557371054,\"since\": "
                                "\"3ktuarhlcjc2z\",\"time\": "
                                "\"2024-06-01T11:33:17.481Z\",\"tooBig\": false}")
                                .object()),
                commit);
        // QVERIFY2(commit.commit == "bafyreibj5esksh55hx3gjdaqcsfmdhbc6kcghhlkj7ikquoddnuwzueqde",
        //          commit.commit.toLocal8Bit());
        QVERIFY2(commit.ops.length() == 1, QString::number(commit.ops.length()).toLocal8Bit());
        QVERIFY2(commit.ops.at(0).action == "create", commit.ops.at(0).action.toLocal8Bit());
        // QVERIFY2(commit.ops.at(0).cid
        //                  == "bafyreihj7nokeio7tw4krnuqmuj6htykx7i4fscgcajbvkrmbuqvh22pee",
        //          commit.ops.at(0).cid.toLocal8Bit());
        QVERIFY2(commit.ops.at(0).path == "app.bsky.feed.post/3ktudvq2ovc2k",
                 commit.ops.at(0).path.toLocal8Bit());
        QVERIFY2(commit.repo == "did:plc:user1", commit.repo.toLocal8Bit());
        QVERIFY2(commit.rev == "3ktudvq2uqs2k", commit.rev.toLocal8Bit());
        QVERIFY2(commit.seq == 557371054, QString::number(commit.seq).toLocal8Bit());
        QVERIFY2(commit.since == "3ktuarhlcjc2z", commit.since.toLocal8Bit());
        QVERIFY2(commit.time == "2024-06-01T11:33:17.481Z", commit.time.toLocal8Bit());
        QVERIFY2(commit.tooBig == false, QString::number(commit.tooBig).toLocal8Bit());
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
#if 0
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
#endif
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
#if 0
    RealtimeFeedListModel model;
    model.setAccount(m_service + "/realtime/1", "did:plc:mqxsuw5b5rhpwo4lw6iwlid5", QString(),
                     QString(), "dummy", QString());

    // model.setSelectorJson("{\"not\":{\"me\":{}}}");
    // model.setSelectorJson("{\"not\":{\"following\":{}}}");
    // model.setSelectorJson("{\"not\":{\"followers\":{}}}");
    model.setSelectorJson("{\"or\": [{\"following\": {}},{\"followers\": {}}]}");
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait(10 * 1000);
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }

    qDebug().noquote() << "---------------------------";
    model.getLatest();

    FirehoseReceiver *recv = FirehoseReceiver::getInstance();
    AbstractPostSelector *s = recv->getSelector(&model);
    QVERIFY(s != nullptr);

    {
        QSignalSpy spy(recv, SIGNAL(connectedToService()));
        spy.wait();
        QVERIFY(spy.count() == 1);
    }

    QVERIFY(s->did() == "did:plc:mqxsuw5b5rhpwo4lw6iwlid5");
    QVERIFY(s->ready() == true);
    QVERIFY(s->needFollowing() == true);
    QVERIFY(s->needFollowers() == true);
#endif
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

QTEST_MAIN(realtime_test)

#include "tst_realtime_test.moc"
