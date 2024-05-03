#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>

#include "cardecoder.h"

int main(int argc, char *argv[])
{
    // QCoreApplication a(argc, argv);
    // return a.exec();

    if (argc != 2) {
        qWarning() << "CAR file decoder";
        qWarning() << "  usage: decodecar path/to/com.atproto.sync.getRepo";
        return 1;
    }

    CarDecoder decoder;
    QFile repo(argv[1]);
    if (!repo.open(QFile::ReadOnly)) {
        qWarning() << "Can not open : " << argv[1];
        return 1;
    }
    decoder.setContent(repo.readAll());
    repo.close();

    QJsonDocument doc;
    QJsonObject root;

    root.insert("header", decoder.headerJson());

    QJsonArray records;
    QJsonArray address;

    QStringList cids;
    QMap<QString, QJsonObject> records_map;

    for (const auto &cid : decoder.cids()) {
        QJsonObject record;
        record.insert("cid", cid);
        record.insert("value", decoder.json(cid));
        if (decoder.type(cid) != "$car_address") {
            record.insert("uri", decoder.uri(cid));
            records.append(record);
        } else {
            address.append(record);
        }
    }

    root.insert("address", address);
    root.insert("records", records);
    doc.setObject(root);

    std::puts(doc.toJson().data());

    return 0;
}
