#ifndef LEXICONS_FUNC_UNKNOWN_H
#define LEXICONS_FUNC_UNKNOWN_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QList>
#include <QString>
#include <QVariant>

namespace LexiconsTypeUnknown {

void copyUnknown(const QJsonObject &src, const QString &property_name, QVariant &dest);

}

#endif // LEXICONS_FUNC_UNKNOWN_H
