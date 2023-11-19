#ifndef LEXICONS_FUNC_UNKNOWN_H
#define LEXICONS_FUNC_UNKNOWN_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QList>
#include <QString>
#include <QVariant>

#include "lexicons.h"

namespace AtProtocolType {
namespace LexiconsTypeUnknown {

void copyUnknown(const QJsonObject &src, QVariant &dest);
void copyBlob(const QJsonObject &src, Blob &dest);

enum class CopyImageType : int {
    Thumb,
    FullSize,
    Alt,
};

QStringList copyImagesFromPostView(const AppBskyFeedDefs::PostView &post, const CopyImageType type);
QStringList copyImagesFromRecord(const AppBskyEmbedRecord::ViewRecord &record,
                                 const CopyImageType type);

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
}

#endif // LEXICONS_FUNC_UNKNOWN_H
