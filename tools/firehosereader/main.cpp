#include <QCoreApplication>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

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
    QUrl url(a.arguments().at(1));
    QString stopper_did = a.arguments().at(2);
    qDebug().noquote() << "url" << url.toString();
    qDebug().noquote() << "Stopper" << stopper_did;
    // wss://bsky.network/xrpc/com.atproto.sync.subscribeRepos

    QStringList skipType;

    skipType << "app.bsky.feed.like"
             << "app.bsky.feed.repost"
             << "app.bsky.graph.follow";

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
                            qDebug().noquote()
                                    << type << json.value("time").toString()
                                    << QDateTime::currentDateTimeUtc().toString(
                                               "yyyy/MM/dd hh:mm:ss.zzz")
                                    << json.value("repo").toString()
                                    << op.toObject().value("action").toString()
                                    << record.value("$type").toString()
                                    << record.value("text").toString().replace("\n", " ").left(30);
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
    client.open(url);

    return a.exec();
}
