#include <QtTest>
#include <QCoreApplication>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonDocument>
#include <QWebSocketServer>

#include "webserver.h"
#include "extension/com/atproto/sync/comatprotosyncsubscribereposex.h"
#include "realtime/abstractpostselector.h"
#include "realtime/firehosereceiver.h"
#include "realtime/realtimefeedlistmodel.h"
#include "realtime/editselectorlistmodel.h"
#include "tools/accountmanager.h"

using namespace RealtimeFeed;
using AtProtocolInterface::ComAtprotoSyncSubscribeReposEx;

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
    void test_FirehoseReceiver_initialCursor();
    void test_FirehoseReceiver_catchingUp();
    void test_JetStreamV2MessageParsing();
    void test_JetStreamV2ErrorFrameCloseOnWorkerThread();
    void test_Websock();
    void test_RealtimeFeedListModel();
    void test_RealtimeFeedListModel_recoverAfterError();
    void test_RealtimeFeedListModel_backfill();
    void test_EditSelectorListModel();
    void test_EditSelectorListModel_append();
    void test_EditSelectorListModel_save();

private:
    QJsonObject makeLikeData(const QString &subject_uri, const QString &subject_cid,
                             const QDateTime &time) const;
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
    m_service = QString("http://127.0.0.1:%1/response").arg(m_listenPort);

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
        // QCOMPARE(spy.count(), 1);
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
        QCOMPARE(following.isEmpty(), false);
        QCOMPARE(followers.isEmpty(), false);

        AbstractPostSelector *root = AbstractPostSelector::create(
                json_item.toObject().value("selector").toObject(), this);
        QCOMPARE_NE(root, nullptr);
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
            QCOMPARE(root->judge(data.toObject().value("payload").toObject()),
                     data.toObject().value("except").toBool());

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

    QCOMPARE(recv->countSelector(), 2);

    const AbstractPostSelector *s1 = recv->getSelector(&parent1);
    const AbstractPostSelector *s2 = recv->getSelector(&parent2);

    QCOMPARE_NE(s1, nullptr);
    QCOMPARE_NE(s2, nullptr);

    QCOMPARE(s1->needFollowers(), false);
    QCOMPARE(s1->needFollowing(), false);
    QCOMPARE(s2->needFollowers(), true);
    QCOMPARE(s2->needFollowing(), false);

    recv->removeSelector(&parent1);
    QCOMPARE(recv->countSelector(), 1);

    QCOMPARE(recv->getSelector(&parent1), nullptr);
    QCOMPARE_NE(recv->getSelector(&parent2), nullptr);

    QCOMPARE(recv->containsSelector(&parent1), false);
    QCOMPARE(recv->containsSelector(&parent2), true);

    // recv->removeAllSelector();
}

void realtime_test::test_FirehoseReceiver_initialCursor()
{
    FirehoseReceiver *recv = FirehoseReceiver::getInstance();
    recv->testResetCursorState();

    // 起動後最初の接続は10分前からのタイムスタンプ(マイクロ秒)
    {
        const qint64 expect = (QDateTime::currentMSecsSinceEpoch() - 10 * 60 * 1000) * 1000;
        const qint64 cursor = recv->testTakeCursor().toLongLong();
        QVERIFY2(qAbs(cursor - expect) < 5 * 1000 * 1000, QString::number(cursor).toLocal8Bit());
    }
    // 受信前に再接続した場合はリアルタイムから
    QCOMPARE(recv->testTakeCursor(), QString());

    // さかのぼる時間の設定
    recv->testResetCursorState();
    recv->setInitialLookbackMinutes(5);
    {
        const qint64 expect = (QDateTime::currentMSecsSinceEpoch() - 5 * 60 * 1000) * 1000;
        const qint64 cursor = recv->testTakeCursor().toLongLong();
        QVERIFY2(qAbs(cursor - expect) < 5 * 1000 * 1000, QString::number(cursor).toLocal8Bit());
    }
    // 0のときはさかのぼらない
    recv->testResetCursorState();
    recv->setInitialLookbackMinutes(0);
    QCOMPARE(recv->testTakeCursor(), QString());
    QCOMPARE(recv->catchingUp(), false);
    recv->setInitialLookbackMinutes(10);

    // 受信後はseqから再開(受信データの時刻が古くても受信した時刻で判断する)
    QJsonObject json;
    json.insert("seq", 24664288881LL);
    json.insert("time", "2026-08-13T06:47:43.959305Z");
    recv->testUpdateReceivedCursorState(json);
    QCOMPARE(recv->testTakeCursor(), QString("24664288882"));

    recv->testResetCursorState();
}

void realtime_test::test_FirehoseReceiver_catchingUp()
{
    FirehoseReceiver *recv = FirehoseReceiver::getInstance();
    recv->testResetCursorState();
    QCOMPARE(recv->catchingUp(), false);
    QVERIFY(recv->testSubscribeCollections().contains("app.bsky.feed.like"));

    // 最初の接続はさかのぼり受信なので追いつくまでいいねを購読しない
    {
        QSignalSpy spy(recv, SIGNAL(catchingUpChanged(bool)));
        QVERIFY(!recv->testTakeCursor().isEmpty());
        QCOMPARE(spy.count(), 1);
        QCOMPARE(recv->catchingUp(), true);
    }
    QCOMPARE(recv->testSubscribeCollections(),
             QStringList() << "app.bsky.feed.post"
                           << "app.bsky.feed.repost"
                           << "app.bsky.graph.follow"
                           << "app.bsky.graph.listitem");

    QJsonObject json;
    // 現在時刻から離れている間は追いつき中のまま
    json.insert("time", QDateTime::currentDateTimeUtc().addSecs(-60).toString(Qt::ISODateWithMs));
    recv->testUpdateCatchingUpState(json);
    QCOMPARE(recv->catchingUp(), true);
    // 現在時刻に近づいたら追いついた
    {
        QSignalSpy spy(recv, SIGNAL(catchingUpChanged(bool)));
        json.insert("time",
                    QDateTime::currentDateTimeUtc().addSecs(-1).toString(Qt::ISODateWithMs));
        recv->testUpdateCatchingUpState(json);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(recv->catchingUp(), false);
    }
    QVERIFY(recv->testSubscribeCollections().contains("app.bsky.feed.like"));

    // 受信前の再接続でリアルタイムからの受信になった場合も追いつき中を解除する
    recv->testResetCursorState();
    recv->testTakeCursor();
    QCOMPARE(recv->catchingUp(), true);
    QCOMPARE(recv->testTakeCursor(), QString());
    QCOMPARE(recv->catchingUp(), false);

    recv->testResetCursorState();
}

void realtime_test::test_JetStreamV2MessageParsing()
{
    ComAtprotoSyncSubscribeReposEx client;

    QSignalSpy receivedSpy(&client, &ComAtprotoSyncSubscribeReposEx::received);
    QSignalSpy errorSpy(&client, &ComAtprotoSyncSubscribeReposEx::errorOccurred);

    QFile fileCreate(":/data/jetstream/commit_create.json");
    QVERIFY(fileCreate.open(QFile::ReadOnly));
    client.testMessageReceivedFromJetStream(fileCreate.readAll());

    QCOMPARE(receivedSpy.count(), 1);
    QCOMPARE(errorSpy.count(), 0);
    {
        QList<QVariant> args = receivedSpy.takeFirst();
        QCOMPARE(args.at(0).toString(), "#commit");
        QJsonObject json = args.at(1).toJsonObject();
        QCOMPARE(json.value("repo").toString(), "did:plc:7e6kocyzb77xkncplrkoojej");
        QCOMPARE(json.value("rev").toString(), "3msx2efqjtc27");
        QCOMPARE(json.value("time").toString(), "2026-08-13T06:47:43.959305Z");
        QCOMPARE(json.value("seq").toVariant().toLongLong(), 24664288881LL);
        QCOMPARE(json.value("commit").toObject().value("$link").toString(),
                 "bafyreigwnxqttkhzha2ig4io6wwht3qiugtor4ruglceyfdbnyq53a55fe");

        QJsonArray ops = json.value("ops").toArray();
        QCOMPARE(ops.count(), 1);
        QCOMPARE(ops.at(0).toObject().value("action").toString(), "create");
        QCOMPARE(ops.at(0).toObject().value("path").toString(), "app.bsky.feed.like/3msx2efqdxs27");
        QCOMPARE(ops.at(0).toObject().value("cid").toObject().value("$link").toString(),
                 "bafyreigwnxqttkhzha2ig4io6wwht3qiugtor4ruglceyfdbnyq53a55fe");

        QJsonArray blocks = json.value("blocks").toArray();
        QCOMPARE(blocks.count(), 1);
        QCOMPARE(blocks.at(0).toObject().value("uri").toString(),
                 "at://did:plc:7e6kocyzb77xkncplrkoojej/app.bsky.feed.like/3msx2efqdxs27");
        QCOMPARE(blocks.at(0).toObject().value("value").toObject().value("$type").toString(),
                 "app.bsky.feed.like");
    }

    // delete: no record -> cid is null and blocks is empty
    QFile fileDelete(":/data/jetstream/commit_delete.json");
    QVERIFY(fileDelete.open(QFile::ReadOnly));
    client.testMessageReceivedFromJetStream(fileDelete.readAll());

    QCOMPARE(receivedSpy.count(), 1);
    QCOMPARE(errorSpy.count(), 0);
    {
        QList<QVariant> args = receivedSpy.takeFirst();
        QJsonObject json = args.at(1).toJsonObject();
        QJsonArray ops = json.value("ops").toArray();
        QCOMPARE(ops.count(), 1);
        QCOMPARE(ops.at(0).toObject().value("action").toString(), "delete");
        QVERIFY(ops.at(0).toObject().value("cid").isNull());
        QCOMPARE(json.value("blocks").toArray().count(), 0);
    }

    // #identity is not handled yet: silently ignored, no signal at all
    QFile fileIdentity(":/data/jetstream/identity.json");
    QVERIFY(fileIdentity.open(QFile::ReadOnly));
    client.testMessageReceivedFromJetStream(fileIdentity.readAll());
    QCOMPARE(receivedSpy.count(), 0);
    QCOMPARE(errorSpy.count(), 0);

    // a server "error" frame is reported and does not emit "received"
    QFile fileError(":/data/jetstream/error.json");
    QVERIFY(fileError.open(QFile::ReadOnly));
    client.testMessageReceivedFromJetStream(fileError.readAll());
    QCOMPARE(receivedSpy.count(), 0);
    QCOMPARE(errorSpy.count(), 1);
    {
        QList<QVariant> args = errorSpy.takeFirst();
        QCOMPARE(args.at(0).toString(), "ConsumerTooSlow");
        QVERIFY(args.at(1).toString().contains("cursor=42"));
    }

    // unreadable JSON is reported as InvalidData
    client.testMessageReceivedFromJetStream(QByteArray("not json"));
    QCOMPARE(receivedSpy.count(), 0);
    QCOMPARE(errorSpy.count(), 1);
    QCOMPARE(errorSpy.takeFirst().at(0).toString(), "InvalidData");
}

void realtime_test::test_JetStreamV2ErrorFrameCloseOnWorkerThread()
{
    // FirehoseReceiverと同じくクライアントを別スレッドへ移した状態で、
    // 受信処理(ワーカースレッド)からの切断がスレッドをまたいだ操作にならないことを確認する
    QTest::failOnWarning(QRegularExpression("another thread|different thread|device not open"));

    QWebSocketServer server("realtime_test", QWebSocketServer::NonSecureMode);
    QVERIFY(server.listen(QHostAddress::LocalHost, 0));
    connect(&server, &QWebSocketServer::newConnection, this, [&server]() {
        QWebSocket *socket = server.nextPendingConnection();
        for (int i = 0; i < 100; i++) {
            socket->sendTextMessage(
                    "{\"$type\":\"message\",\"payload\":{\"$type\":\"x#identity\"}}");
        }
        // サーバー側からは切断せず、クライアント側のclose()で切断させる
        socket->sendTextMessage(
                "{\"$type\":\"error\",\"error\":\"ConsumerTooSlow\",\"message\":\"test\"}");
    });

    ComAtprotoSyncSubscribeReposEx client;
    QThread thread;
    client.moveToThread(&thread);
    thread.start();

    // ワーカースレッドから発行されるシグナルをこのスレッドで受ける
    QStringList errors;
    int disconnected = 0;
    connect(&client, &ComAtprotoSyncSubscribeReposEx::errorOccurred, this,
            [&errors](const QString &code, const QString &) { errors.append(code); });
    connect(&client, &ComAtprotoSyncSubscribeReposEx::disconnectFromService, this,
            [&disconnected]() { disconnected++; });

    client.open(QUrl(QString("ws://127.0.0.1:%1").arg(server.serverPort())),
                ComAtprotoSyncSubscribeReposEx::SubScribeMode::JetStream);

    QTRY_COMPARE_WITH_TIMEOUT(disconnected, 1, 10 * 1000);
    QCOMPARE(errors, QStringList() << "ConsumerTooSlow");
    QTRY_COMPARE(client.state(), QAbstractSocket::UnconnectedState);

    thread.quit();
    thread.wait();
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
        QCOMPARE(spy.count(), 1);
    }
    {
        QSignalSpy spy(recv->getSelector(&parent1), SIGNAL(selected(const QJsonObject &)));
        spy.wait(60 * 1000);
        qDebug().noquote() << "selected : spy.count" << spy.count();
        QCOMPARE(spy.count() , 1);
    }
    {
        QSignalSpy spy(recv, SIGNAL(disconnectFromService()));
        qDebug().noquote() << "before removeSelector";
        recv->removeSelector(&parent1);
        spy.wait();
        QCOMPARE(spy.count(), 1);
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
                          "{\"uri\":\"at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.graph.list/"
                          "3kcj52ovctd2h\",\"name\":\"My Accounts\"}}]}");
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait(20 * 1000);
        QCOMPARE(spy.count(), 2);
    }

    qDebug().noquote() << "---------------------------";
    model.getLatest();

    FirehoseReceiver *recv = FirehoseReceiver::getInstance();
    AbstractPostSelector *s = recv->getSelector(&model);
    QCOMPARE_NE(s, nullptr);

    {
        QSignalSpy spy(recv, SIGNAL(connectedToService()));
        recv->start();
        spy.wait();
        QCOMPARE(spy.count(), 1);
    }

    QCOMPARE(s->did(), "did:plc:mqxsuw5b5rhpwo4lw6iwlid5");
    QCOMPARE(s->ready(), true);
    QCOMPARE(s->needFollowing(), true);
    QCOMPARE(s->needFollowers(), true);

    QJsonDocument json_doc;

    qDebug().noquote() << "---------------------------";
    json_doc = loadJson(":/data/realtimemodel/recv_data_1.json");
    QVERIFY(json_doc.isObject());
    QCOMPARE(model.rowCount(), 0);
    {
        QSignalSpy spy(&model, SIGNAL(rowsInserted(const QModelIndex &, int, int)));
        recv->testReceived(json_doc.object());
        spy.wait();
        QCOMPARE(spy.count(), 1);
    }
    QCOMPARE(model.rowCount(), 1);

    QVERIFY(model.item(0, TimelineListModel::CidRole).toString()
            == "bafyreigoon4vpg3axqlvrzyxcpmwh4ihra4hbqd5uh3e774bbjjnla5ajq");
    QCOMPARE(model.item(0, TimelineListModel::RecordTextPlainRole).toString(), "reply3");
    // リプライ先は同じgetPostsのリクエストで取得する
    {
        const QList<OperationInfo> infos = s->getOperationInfos(json_doc.object());
        QCOMPARE(infos.count(), 1);
        QCOMPARE(infos.first().reply_parent_uri,
                 "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3kx6eow2kyr25");
    }
    QCOMPARE(model.item(0, TimelineListModel::HasReplyRole).toBool(), true);
    QCOMPARE(model.item(0, TimelineListModel::ReplyParentDisplayNameRole).toString(), "iori2");
    QCOMPARE(model.item(0, TimelineListModel::ReplyParentHandleRole).toString(),
             "ioriayane2.bsky.social");

    // ポスト取得より前のリアクションは取得したカウントに含まれているので加算しない
    {
        const QString uri =
                "at://did:plc:l4fsx4ujos7uw7n4ijq2ulgs/app.bsky.feed.post/3kx6ffqxzkj2u";
        const QString cid = "bafyreigoon4vpg3axqlvrzyxcpmwh4ihra4hbqd5uh3e774bbjjnla5ajq";
        const int like_count = model.item(0, TimelineListModel::LikeCountRole).toInt();
        s->judgeSelectionAndReaction(
                makeLikeData(uri, cid, QDateTime::currentDateTimeUtc().addSecs(-10 * 60)));
        QCOMPARE(model.item(0, TimelineListModel::LikeCountRole).toInt(), like_count);
        // 取得後のリアクションは反映する
        s->judgeSelectionAndReaction(
                makeLikeData(uri, cid, QDateTime::currentDateTimeUtc().addSecs(10 * 60)));
        QCOMPARE(model.item(0, TimelineListModel::LikeCountRole).toInt(), like_count + 1);
    }

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
        QCOMPARE(spy.count(), 1);
    }
    QCOMPARE(model.rowCount(), 2);

    QVERIFY(model.item(0, TimelineListModel::CidRole).toString()
            == "bafyreiadfsi4feaygror6ekk5j7mlhquau7zbf3jlxasmjjcsdjs75zbjq");
    QCOMPARE(model.item(0, TimelineListModel::RecordTextPlainRole).toString(), "update hagoromo");
    QCOMPARE(model.item(0, TimelineListModel::IsRepostedByRole).toBool(), true);
    QCOMPARE(model.item(0, TimelineListModel::RepostedByDisplayNameRole).toString(), "");
    QVERIFY(model.item(0, TimelineListModel::RepostedByHandleRole).toString()
            == "ioriayane.bsky.social");

    QVERIFY(model.item(1, TimelineListModel::CidRole).toString()
            == "bafyreigoon4vpg3axqlvrzyxcpmwh4ihra4hbqd5uh3e774bbjjnla5ajq");
    QCOMPARE(model.item(1, TimelineListModel::RecordTextPlainRole).toString(), "reply3");
    QCOMPARE(model.item(1, TimelineListModel::IsRepostedByRole).toBool(), false);
    QCOMPARE(model.item(0, TimelineListModel::HasReplyRole).toBool(), false);

    // /////////////////

    qDebug().noquote() << "---------------------------";
    uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/realtime/3", "id", "pass", "did:plc:mqxsuw5b5rhpwo4lw6iwlid5",
            "handle", "email", "accessJwt", "refreshJwt", true);
    model.setAccount(uuid);
    json_doc = loadJson(":/data/realtimemodel/recv_data_3.json");
    recv->testReceived(json_doc.object());
    QCOMPARE(model.rowCount(), 2);

    qDebug().noquote() << "---------------------------";
    json_doc = loadJson(":/data/realtimemodel/recv_data_4.json");
    recv->testReceived(json_doc.object());
    QCOMPARE(model.rowCount(), 2);

    qDebug().noquote() << "---------------------------";
    json_doc = loadJson(":/data/realtimemodel/recv_data_3.json");
    QVERIFY(json_doc.isObject());
    {
        QSignalSpy spy(&model, SIGNAL(rowsInserted(const QModelIndex &, int, int)));
        recv->testReceived(json_doc.object());
        spy.wait();
        QCOMPARE(spy.count(), 1);
    }
    QCOMPARE(model.rowCount(), 3);
    QVERIFY(model.item(0, TimelineListModel::CidRole).toString()
            == "bafyreigoon4vpg3axqlvrzyxcpmwh4ihra4hbqd5uh3e774bbjjnla5ajq3");
    QCOMPARE(model.item(0, TimelineListModel::RecordTextPlainRole).toString(), "post 3");
    QCOMPARE(model.item(0, TimelineListModel::IsRepostedByRole).toBool(), false);

    // ///////////////////

    qDebug().noquote() << "---------------------------";
    uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/realtime/4", "id", "pass", "did:plc:mqxsuw5b5rhpwo4lw6iwlid5",
            "handle", "email", "accessJwt", "refreshJwt", true);
    model.setAccount(uuid);
    json_doc = loadJson(":/data/realtimemodel/recv_data_5.json");
    recv->testReceived(json_doc.object());
    QCOMPARE(model.rowCount(), 3);

    qDebug().noquote() << "---------------------------";
    json_doc = loadJson(":/data/realtimemodel/recv_data_6.json");
    recv->testReceived(json_doc.object());
    QCOMPARE(model.rowCount(), 3);

    qDebug().noquote() << "---------------------------";
    json_doc = loadJson(":/data/realtimemodel/recv_data_5.json");
    QVERIFY(json_doc.isObject());
    {
        QSignalSpy spy(&model, SIGNAL(rowsInserted(const QModelIndex &, int, int)));
        recv->testReceived(json_doc.object());
        spy.wait();
        QCOMPARE(spy.count(), 1);
    }
    QCOMPARE(model.rowCount(), 4);
    QVERIFY(model.item(0, TimelineListModel::CidRole).toString()
            == "bafyreigoon4vpg3axqlvrzyxcpmwh4ihra4hbqd5uh3e774bbjjnla5ajq5");
    QCOMPARE(model.item(0, TimelineListModel::RecordTextPlainRole).toString(), "post 5");
    QCOMPARE(model.item(0, TimelineListModel::IsRepostedByRole).toBool(), false);
}

void realtime_test::test_RealtimeFeedListModel_recoverAfterError()
{
    // getLatest() を初めて実行したときの一連の処理
    //（following/followers/list の取得 -> finishGetting()）の途中でエラーが
    // 発生した場合に、次回の getLatest() で最初からやり直せることを確認する。

    FirehoseReceiver *recv = FirehoseReceiver::getInstance();

    // getFollows のレスポンスが用意されていないサービスを使ってエラーを起こす
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/realtime/error", "id", "pass",
            "did:plc:mqxsuw5b5rhpwo4lw6iwlid5", "handle", "email", "accessJwt", "refreshJwt", true);

    RealtimeFeedListModel model;
    model.setAccount(uuid);
    model.setSelectorJson("{\"following\": {}}");

    QCOMPARE(recv->containsSelector(&model), false);

    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait(20 * 1000);
    }

    // finishGetting() に到達できなかったので、不完全な selector が残っていない
    // （= 次回 getLatest() が最初からやり直せる状態になっている）ことを確認する
    QCOMPARE(recv->containsSelector(&model), false);
    QCOMPARE(model.receiving(), false);

    // 同じ selectorJson のまま、正常にレスポンスが返るサービスへ切り替えて再実行する
    uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/realtime/1", "id", "pass", "did:plc:mqxsuw5b5rhpwo4lw6iwlid5",
            "handle", "email", "accessJwt", "refreshJwt", true);
    model.setAccount(uuid);

    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait(20 * 1000);
    }

    AbstractPostSelector *s = recv->getSelector(&model);
    QCOMPARE_NE(s, nullptr);
    QCOMPARE(s->ready(), true);
    QCOMPARE(recv->selectorIsReady(&model), true);

    recv->removeSelector(&model);
}

void realtime_test::test_RealtimeFeedListModel_backfill()
{
    FirehoseReceiver *recv = FirehoseReceiver::getInstance();
    recv->testResetCursorState();

    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/realtime/1", "id", "pass", "did:plc:mqxsuw5b5rhpwo4lw6iwlid5",
            "handle", "email", "accessJwt", "refreshJwt", true);

    RealtimeFeedListModel model;
    model.setAccount(uuid);
    model.setSelectorJson("{\"or\": [{\"following\": {}},{\"followers\": {}}]}");
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait(20 * 1000);
        QCOMPARE(spy.count(), 2);
    }
    QCOMPARE(recv->selectorIsReady(&model), true);
    QCOMPARE(model.running(), false);

    // 追いつくまでは取得せずに溜めておく
    recv->testSetCatchingUp(true);
    QCOMPARE(model.running(), true);
    const QJsonDocument json_doc = loadJson(":/data/realtimemodel/recv_data_1.json");
    QVERIFY(json_doc.isObject());
    for (int i = 0; i < 150; i++) {
        recv->testReceived(json_doc.object());
    }
    QTest::qWait(500);
    QCOMPARE(model.rowCount(), 0);

    // 追いついたら新しいものから上限件数まで取得する
    recv->testSetCatchingUp(false);
    QCOMPARE(model.running(), false);
    QTRY_COMPARE_WITH_TIMEOUT(model.rowCount(), 100, 10 * 1000);
    QTest::qWait(500);
    QCOMPARE(model.rowCount(), 100);

    recv->testResetCursorState();
}

void realtime_test::test_EditSelectorListModel()
{
    EditSelectorListModel model;

    QCOMPARE(model.rowCount(), 0);
    QVERIFY(model.item(0, EditSelectorListModel::EditSelectorListModelRoles::TypeRole)
            == QVariant());
    QCOMPARE(model.valid(), false);

    //----
    model.setSelectorJson("{\"me\":{}}");
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.valid(), true);
    QCOMPARE(model.item(0, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString(),
             "me");
    QVERIFY(model.item(1, EditSelectorListModel::EditSelectorListModelRoles::TypeRole)
            == QVariant());

    QCOMPARE(model.item(0, EditSelectorListModel::EditSelectorListModelRoles::IndentRole).toInt(),
             0);

    //----
    model.setSelectorJson("{\"not\":{\"me\":{}}}");
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.valid(), true);
    QCOMPARE(model.item(0, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString(),
             "not");
    QCOMPARE(model.item(1, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString(),
             "me");
    QCOMPARE(model.item(2, EditSelectorListModel::EditSelectorListModelRoles::TypeRole),
             QVariant());

    QCOMPARE(model.item(0, EditSelectorListModel::EditSelectorListModelRoles::IndentRole).toInt(),
             0);
    QCOMPARE(model.item(1, EditSelectorListModel::EditSelectorListModelRoles::IndentRole).toInt(),
             1);

    //----
    model.setSelectorJson("{\"or\": [{\"following\": {}},{\"followers\": {}}]}");
    QCOMPARE(model.rowCount(), 3);
    QCOMPARE(model.valid(), true);
    QCOMPARE(model.item(0, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString(),
             "or");
    QCOMPARE(model.item(1, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString(),
             "following");
    QCOMPARE(model.item(2, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString(),
             "followers");
    QCOMPARE(model.item(3, EditSelectorListModel::EditSelectorListModelRoles::TypeRole),
             QVariant());

    QCOMPARE(model.item(0, EditSelectorListModel::EditSelectorListModelRoles::IndentRole).toInt(),
             0);
    QCOMPARE(model.item(1, EditSelectorListModel::EditSelectorListModelRoles::IndentRole).toInt(),
             1);
    QCOMPARE(model.item(2, EditSelectorListModel::EditSelectorListModelRoles::IndentRole).toInt(),
             1);

    //----
    model.setSelectorJson("{\"and\": [{\"followers\": {}},{\"following\": {}},{\"me\":{}}]}");
    QCOMPARE(model.rowCount(), 4);
    QCOMPARE(model.valid(), true);
    QCOMPARE(model.item(0, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString(),
             "and");
    QCOMPARE(model.item(1, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString(),
             "followers");
    QCOMPARE(model.item(2, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString(),
             "following");
    QCOMPARE(model.item(3, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString(),
             "me");
    QVERIFY(model.item(4, EditSelectorListModel::EditSelectorListModelRoles::TypeRole)
            == QVariant());

    QCOMPARE(model.item(0, EditSelectorListModel::EditSelectorListModelRoles::IndentRole).toInt(),
             0);
    QCOMPARE(model.item(1, EditSelectorListModel::EditSelectorListModelRoles::IndentRole).toInt(),
             1);
    QCOMPARE(model.item(2, EditSelectorListModel::EditSelectorListModelRoles::IndentRole).toInt(),
             1);
    QCOMPARE(model.item(3, EditSelectorListModel::EditSelectorListModelRoles::IndentRole).toInt(),
             1);

    //----
    model.setSelectorJson("{\"and\": [{\"not\": {\"following\": {}}},{\"xor\": [{\"following\": "
                          "{}},{\"not\": {\"followers\": {}}}]},{\"following\": {}}]}");
    QCOMPARE(model.rowCount(), 8);
    QCOMPARE(model.valid(), true);
    QCOMPARE(model.item(0, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString(),
             "and");
    QCOMPARE(model.item(1, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString(),
             "not");
    QCOMPARE(model.item(2, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString(),
             "following");
    QCOMPARE(model.item(3, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString(),
             "xor");
    QCOMPARE(model.item(4, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString(),
             "following");
    QCOMPARE(model.item(5, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString(),
             "not");
    QCOMPARE(model.item(6, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString(),
             "followers");
    QCOMPARE(model.item(7, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString(),
             "following");
    QVERIFY(model.item(8, EditSelectorListModel::EditSelectorListModelRoles::TypeRole)
            == QVariant());

    QCOMPARE(model.item(0, EditSelectorListModel::EditSelectorListModelRoles::IndentRole).toInt(),
             0);
    QCOMPARE(model.item(1, EditSelectorListModel::EditSelectorListModelRoles::IndentRole).toInt(),
             1);
    QCOMPARE(model.item(2, EditSelectorListModel::EditSelectorListModelRoles::IndentRole).toInt(),
             2);
    QCOMPARE(model.item(3, EditSelectorListModel::EditSelectorListModelRoles::IndentRole).toInt(),
             1);
    QCOMPARE(model.item(4, EditSelectorListModel::EditSelectorListModelRoles::IndentRole).toInt(),
             2);
    QCOMPARE(model.item(5, EditSelectorListModel::EditSelectorListModelRoles::IndentRole).toInt(),
             2);
    QCOMPARE(model.item(6, EditSelectorListModel::EditSelectorListModelRoles::IndentRole).toInt(),
             3);
    QCOMPARE(model.item(7, EditSelectorListModel::EditSelectorListModelRoles::IndentRole).toInt(),
             1);

    QCOMPARE(model.item(0, EditSelectorListModel::EditSelectorListModelRoles::DisplayTypeRole)
                     .toString(),
             "AND");
    QCOMPARE(model.item(6, EditSelectorListModel::EditSelectorListModelRoles::DisplayTypeRole)
                     .toString(),
             "Followers");

    QCOMPARE(model.item(0, EditSelectorListModel::EditSelectorListModelRoles::CanHaveRole).toBool(),
             true);
    QCOMPARE(model.item(6, EditSelectorListModel::EditSelectorListModelRoles::CanHaveRole).toBool(),
             false);
}

void realtime_test::test_EditSelectorListModel_append()
{
    EditSelectorListModel model;

    model.appendChild(-1, "me");
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.valid(), true);
    QCOMPARE(model.item(0, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString(),
             "me");

    // don't append
    model.appendChild(0, "not");
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.valid(), true);

    model.clear();

    qDebug() << "----";

    model.appendChild(-1, "and");
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.valid(), false);
    QCOMPARE(model.item(0, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString(),
             "and");

    model.appendChild(0, "following");
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.valid(), true);
    QCOMPARE(model.item(1, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString(),
             "following");

    model.appendChild(0, "or");
    QCOMPARE(model.rowCount(), 3);
    QCOMPARE(model.valid(), false);
    QCOMPARE(model.item(2, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString(),
             "or");

    model.appendChild(2, "followers");
    QCOMPARE(model.rowCount(), 4);
    QCOMPARE(model.valid(), true);
    QCOMPARE(model.item(3, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString(),
             "followers");

    model.appendChild(0, "xor");
    QCOMPARE(model.rowCount(), 5);
    QCOMPARE(model.valid(), false);
    QCOMPARE(model.item(4, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString(),
             "xor");

    model.appendChild(2, "following");
    QCOMPARE(model.rowCount(), 6);
    QCOMPARE(model.valid(), false);
    QCOMPARE(model.item(4, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString(),
             "following");
    QCOMPARE(model.item(5, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString(),
             "xor");

    model.remove(2);
    QCOMPARE(model.rowCount(), 3);
    QCOMPARE(model.valid(), false);

    model.appendChild(2, "following");
    QCOMPARE(model.rowCount(), 4);
    QCOMPARE(model.valid(), true);
    QCOMPARE(model.item(3, EditSelectorListModel::EditSelectorListModelRoles::TypeRole).toString(),
             "following");

    // qDebug().noquote() << model.toJson();

    model.remove(0);
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.valid(), false);
}

void realtime_test::test_EditSelectorListModel_save()
{

    EditSelectorListModel model;

    model.appendChild(-1, "me");
    QCOMPARE(model.count(), 1);
    model.update(0, EditSelectorListModel::HasImageRole, true);
    QCOMPARE(model.item(0, EditSelectorListModel::HasImageRole).toBool(), true);
    model.update(0, EditSelectorListModel::ImageCountRole, 3);
    QCOMPARE(model.item(0, EditSelectorListModel::ImageCountRole).toInt(), 3);
    QCOMPARE(model.toJson(), "{\"me\":{\"image\":{\"has\":true,\"count\":3}}}");
    model.update(0, EditSelectorListModel::HasImageRole, false);
    QCOMPARE(model.item(0, EditSelectorListModel::HasImageRole).toBool(), false);
    QCOMPARE(model.toJson(), "{\"me\":{}}");

    model.update(0, EditSelectorListModel::HasMovieRole, true);
    QCOMPARE(model.item(0, EditSelectorListModel::HasMovieRole).toBool(), true);
    QCOMPARE(model.toJson(), "{\"me\":{\"movie\":{\"has\":true,\"count\":0}}}");
    model.update(0, EditSelectorListModel::HasMovieRole, false);
    QCOMPARE(model.item(0, EditSelectorListModel::HasMovieRole).toBool(), false);
    QCOMPARE(model.toJson(), "{\"me\":{}}");

    qDebug().noquote() << model.toJson();

    model.update(0, EditSelectorListModel::HasQuoteRole, true);
    QCOMPARE(model.item(0, EditSelectorListModel::HasQuoteRole).toBool(), true);
    QCOMPARE(model.toJson(), "{\"me\":{\"quote\":{\"has\":true,\"condition\":0}}}");
    model.update(0, EditSelectorListModel::HasQuoteRole, false);
    QCOMPARE(model.item(0, EditSelectorListModel::HasQuoteRole).toBool(), false);

    model.update(0, EditSelectorListModel::HasImageRole, true);
    model.update(0, EditSelectorListModel::HasMovieRole, true);
    model.update(0, EditSelectorListModel::HasQuoteRole, true);
    model.update(0, EditSelectorListModel::IsRepostRole, true);
    QCOMPARE(model.item(0, EditSelectorListModel::HasImageRole).toBool(), true);
    QCOMPARE(model.item(0, EditSelectorListModel::HasMovieRole).toBool(), true);
    QCOMPARE(model.item(0, EditSelectorListModel::HasQuoteRole).toBool(), true);
    QCOMPARE(model.item(0, EditSelectorListModel::IsRepostRole).toBool(), true);
    QCOMPARE(model.toJson(),
             "{\"me\":{\"image\":{\"has\":true,\"count\":3},"
             "\"movie\":{\"has\":true,"
             "\"count\":0},\"quote\":{\"has\":true,\"condition\":0},"
             "\"repost\":{\"is\":true,\"condition\":0}}}");

    qDebug().noquote() << model.toJson();
    qDebug() << model.toJson();

    model.clear();
    model.appendChild(-1, "list");
    QCOMPARE(model.count(), 1);

    QString uri = "at://did:plc:ipj5qejfoqu6eukvt72uhyit/app.bsky.graph.list/3kgybwfpfag2i";
    QString name = "illust";
    model.update(0, EditSelectorListModel::ListUriRole, uri);
    model.update(0, EditSelectorListModel::ListNameRole, name);
    QCOMPARE(model.item(0, EditSelectorListModel::ListUriRole).toString(), uri);
    QCOMPARE(model.item(0, EditSelectorListModel::ListNameRole).toString(), name);
    QVERIFY(model.toJson()
            == QString("{\"list\":{\"uri\":\"%1\",\"name\":\"%2\"}}").arg(uri, name));

    QString json;

    json = "{\"or\":[{\"and\":[{\"following\":{\"image\":{\"has\":true,\"count\":4}}"
           "},{\"followers\":{\"movie\":{\"has\":true,\"count\":0}}}]},{\"me\":{}},{\"not\":{"
           "\"following\":{\"quote\":{\"has\":true,\"condition\":1},"
           "\"repost\":{\"is\":true,\"condition\":1}}}}]}";
    model.clear();
    model.setSelectorJson(json);
    QCOMPARE(model.toJson(), json);

    json = "{\"and\":[{\"following\":{}},"
           "{\"list\":{\"uri\":\"at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.graph.list/"
           "3kflf2r3lwg2x\",\"name\":\"ice\"}}]}";
    model.clear();
    model.setSelectorJson(json);
    QCOMPARE(model.toJson(), json);

    json = "{\"and\":[{\"list\":{\"uri\":\"at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/"
           "app.bsky.graph.list/3kflf2r3lwg2x\",\"name\":\"ice\"}},{\"list\":{\"uri\":\"at://"
           "did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.graph.list/"
           "3kflbnc4c4o2x\",\"name\":\"list\"}},{\"or\":[{\"list\":{\"uri\":\"at://"
           "did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.graph.list/"
           "3kflf2r3lwg2x\",\"name\":\"ice\"}}]}]}";

    QJsonObject json_obj = QJsonDocument::fromJson(json.toUtf8()).object();
    QCOMPARE(json_obj.isEmpty(), false);
    AbstractPostSelector *selector = AbstractPostSelector::create(json_obj, this);
    QCOMPARE_NE(selector, nullptr);
    QCOMPARE(selector->toString(), json);
    QVERIFY(selector->getListUris()
            == QStringList()
                    << "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.graph.list/3kflf2r3lwg2x"
                    << "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.graph.list/3kflbnc4c4o2x");
    selector->deleteLater();
}

QJsonObject realtime_test::makeLikeData(const QString &subject_uri, const QString &subject_cid,
                                        const QDateTime &time) const
{
    const QString repo = "did:plc:ipj5qejfoqu6eukvt72uhyit";
    const QString path = "app.bsky.feed.like/3kx6ffqxzkj3a";
    const QString cid = "bafyreihr2hrmavhzdpmnc65udreph5vfmd3xceqtw2jm3b4ffkagedoqzm";

    QJsonObject subject;
    subject.insert("uri", subject_uri);
    subject.insert("cid", subject_cid);
    QJsonObject value;
    value.insert("$type", "app.bsky.feed.like");
    value.insert("createdAt", time.toString(Qt::ISODateWithMs));
    value.insert("subject", subject);
    QJsonObject block;
    block.insert("cid", cid);
    block.insert("uri", QString("at://%1/%2").arg(repo, path));
    block.insert("value", value);
    QJsonArray blocks;
    blocks.append(block);

    QJsonObject op_cid;
    op_cid.insert("$link", cid);
    QJsonObject op;
    op.insert("action", "create");
    op.insert("cid", op_cid);
    op.insert("path", path);
    QJsonArray ops;
    ops.append(op);

    QJsonObject json;
    json.insert("repo", repo);
    json.insert("time", time.toString(Qt::ISODateWithMs));
    json.insert("blocks", blocks);
    json.insert("ops", ops);
    return json;
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
