#include <QCoreApplication>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QElapsedTimer>
#include <QUrl>

#include "comatprotosyncsubscribereposex.h"

using AtProtocolInterface::ComAtprotoSyncSubscribeReposEx;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (a.arguments().count() < 3) {
        qWarning() << "Firehose reader";
        qWarning() << "  usage: firehosereader URL STOPER_USER_DID";
        return 1;
    }
    ComAtprotoSyncSubscribeReposEx::SubScribeMode mode =
            ComAtprotoSyncSubscribeReposEx::SubScribeMode::Firehose;
    QUrl url(a.arguments().at(1));
    QString stopper_did = a.arguments().at(2);
    qDebug().noquote() << "url" << url.toString();
    qDebug().noquote() << "  " << url.scheme();
    qDebug().noquote() << "  " << url.host();
    qDebug().noquote() << "  " << url.path();
    qDebug().noquote() << "  " << url.query();
    qDebug().noquote() << "Stopper" << stopper_did;
    // wss://bsky.network/xrpc/com.atproto.sync.subscribeRepos
    // wss://jetstream2.us-west.bsky.network/subscribe?wantedCollections=app.bsky.feed.post&wantedCollections=app.bsky.feed.repost&wantedCollections=app.bsky.graph.follow

    if (url.host().startsWith("jetstream")) {
        mode = ComAtprotoSyncSubscribeReposEx::SubScribeMode::JetStream;
    }

    QElapsedTimer timer;
    timer.start();

    QStringList skipType;

    skipType << "app.bsky.feed.like"
             << "app.bsky.feed.repost"
             << "app.bsky.graph.follow"
             << "app.bsky.graph.block"
             << "app.bsky.actor.profile"
             << "app.bsky.graph.listitem"
             << "chat.bsky.actor.declaration";

    ComAtprotoSyncSubscribeReposEx client;
    QObject::connect(&client, &ComAtprotoSyncSubscribeReposEx::errorOccured,
                     [](const QString &error, const QString &message) {
                         qDebug().noquote() << "Error:" << error << message;
                         QCoreApplication::quit();
                     });
    QObject::connect(
            &client, &ComAtprotoSyncSubscribeReposEx::received,
            [=](const QString &type, const QJsonObject &json) {
                // qDebug().noquote() << "commitDataReceived:" << !json.isEmpty();
                static unsigned long count = 0;
                static qint64 prev_time = 0;
                qint64 cur_time = timer.elapsed();
                if ((cur_time - prev_time) > 1000) {
                    qDebug().noquote() << "rate:" << count; // << static_cast<int>(count / 10);
                    count = 0;
                    prev_time = cur_time;
                } else {
                    count++;
                }
                if (type == "#commit") {
                    for (const auto &op : json.value("ops").toArray()) {
                        if (op.toObject().value("path").toString().startsWith(
                                    "app.bsky.feed.like/")) {
                            continue;
                        }
                        for (const auto &block : json.value("blocks").toArray()) {
                            QJsonObject record = block.toObject().value("value").toObject();
                            if (skipType.contains(record.value("$type").toString())) {
                                continue;
                            }
                            QDateTime date = QDateTime::fromString(json.value("time").toString(),
                                                                   Qt::ISODateWithMs);
                            qDebug().noquote()
                                    << type << json.value("time").toString()
                                    << date.msecsTo(QDateTime::currentDateTimeUtc())
                                    << json.value("repo").toString()
                                    << op.toObject().value("action").toString()
                                    << record.value("$type").toString()
                                    << record.value("text").toString().replace("\n", " ").left(30);
                            if (record.contains("via")) {
                                qDebug().noquote() << "                                        "
                                                   << "via:" << record.value("via").toString();
                            }
                        }
                    }

                    // } else {
                    //     qDebug().noquote()
                    //             << type << json.value("time").toString()
                    //             << QDateTime::currentDateTimeUtc().toString("yyyy/MM/dd
                    //             hh:mm:ss.zzz")
                    //             << json.value("repo").toString()
                    //             << json.value("ops")
                    //                        .toArray()
                    //                        .first()
                    //                        .toObject()
                    //                        .value("action")
                    //                        .toString();
                    //     qDebug().noquote() << QJsonDocument(json).toJson();
                }

                if (json.value("repo").toString() == stopper_did) {
                    qDebug() << "-- Stop --";
                    qDebug().noquote() << QJsonDocument(json).toJson();
                    QCoreApplication::quit();
                }
            });
    client.open(url, mode);

    QNetworkRequest req(url);
    qDebug().noquote() << req.url().toString() << req.url().query();

    return a.exec();
}
