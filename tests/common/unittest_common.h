#ifndef UNITTEST_COMMON_H
#define UNITTEST_COMMON_H

#include <QString>
#include <QJsonObject>
#include <QHash>

namespace UnitTestCommon {
QJsonDocument loadJson(const QString &path);
QHash<QString, QString> loadPostHash(const QString &path);
QHash<QString, QJsonObject> loadPostExpectHash(const QString &path);
QHash<QString, QJsonObject> loadVoteHash(const QString &path);
}

#endif // UNITTEST_COMMON_H
