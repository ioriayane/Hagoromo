#ifndef LEXICONS_FUNC_UNKNOWN_H
#define LEXICONS_FUNC_UNKNOWN_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QList>
#include <QString>
#include <QVariant>

namespace LexiconsTypeUnknown {

void copyUnknown(const QJsonObject &src, QVariant &dest);

template<typename T>
T fromQVariant(const QVariant &variant)
{
    if (variant.canConvert<T>()) {
        return variant.value<T>();
    } else {
        return T();
    }
}

}

#endif // LEXICONS_FUNC_UNKNOWN_H