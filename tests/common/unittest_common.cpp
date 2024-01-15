#include "unittest_common.h"

#include <QFile>
#include <QJsonDocument>

namespace UnitTestCommon {

QJsonDocument loadJson(const QString &path)
{
    QFile file(path);
    if (file.open(QFile::ReadOnly)) {
        return QJsonDocument::fromJson(file.readAll());
    } else {
        return QJsonDocument();
    }
}

QHash<QString, QString> loadPostHash(const QString &path)
{
    QHash<QString, QString> hash;
    QJsonDocument json_doc = loadJson(path);
    for (const auto &key : json_doc.object().keys()) {
        hash[key] = json_doc.object().value(key).toObject().value("text").toString();
    }
    return hash;
}

QHash<QString, QJsonObject> loadPostExpectHash(const QString &path)
{
    QHash<QString, QJsonObject> hash;
    QJsonDocument json_doc = loadJson(path);
    for (const auto &key : json_doc.object().keys()) {
        hash[key] = json_doc.object().value(key).toObject().value("expect").toObject();
    }
    return hash;
}

}
