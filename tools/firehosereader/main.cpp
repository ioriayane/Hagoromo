#include <QCoreApplication>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCborMap>

#include "comatprotosyncsubscribereposex.h"
#include "atprotocol/lexicons_func_unknown.h"
#include "tools/cardecoder.h"

using AtProtocolInterface::ComAtprotoSyncSubscribeReposEx;
using namespace AtProtocolType;

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
            [=](const QString &type, const QCborValue &cbor) {
                // qDebug().noquote() << "commitDataReceived:" << !json.isEmpty();

                ComAtprotoSyncSubscribeRepos::Commit commit;
                ComAtprotoSyncSubscribeRepos::copyCommit(cbor, commit);

                if (type == "#commit") {
                    CarDecoder decorder;
                    if (!commit.blocks.isEmpty()) {
                        if (decorder.setContent(commit.blocks)) { }
                    }
                    for (const auto &op : commit.ops) {
                        if (op.path.startsWith("app.bsky.feed.like/")) {
                            continue;
                        }

                        QString op_cid;
                        {
                            int offset = 0;
                            op_cid = CarDecoder::decodeCid(op.cid, offset);
                        }
                        if (decorder.cids().contains(op_cid)) {
                            QJsonObject record = decorder.json(op_cid);
                            if (skipType.contains(record.value("$type").toString())) {
                                continue;
                            }
                            qDebug().noquote()
                                    << type << commit.time
                                    << QDateTime::currentDateTimeUtc().toString(
                                               "yyyy/MM/dd hh:mm:ss.zzz")
                                    << commit.repo << op.action << record.value("$type").toString()
                                    << record.value("text").toString().replace("\n", "").left(30);

                        } else {
                            qDebug().noquote() << type << commit.time
                                               << QDateTime::currentDateTimeUtc().toString(
                                                          "yyyy/MM/dd hh:mm:ss.zzz")
                                               << commit.repo << op.path;
                        }
                    }

                    // } else {
                    //     qDebug().noquote()
                    //             << type << commit.time << commit.repo
                    //             << (commit.ops.isEmpty() ? "" :
                    //             commit.ops.first().action);
                    // qDebug().noquote() << QJsonDocument(json).toJson();
                }

                if (commit.repo == stopper_did) {
                    qDebug() << "-- Stop --";
                    qDebug().noquote() << QJsonDocument(cbor.toJsonValue().toObject()).toJson();
                    QCoreApplication::quit();
                }
            });
    client.open(url);

    return a.exec();
}
