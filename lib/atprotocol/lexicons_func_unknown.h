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

struct Blob
{
    QString cid;
    QString mimeType;
    int size = 0;
};

void copyUnknown(const QJsonObject &src, QVariant &dest);

QStringList copyImagesFromPostView(const AppBskyFeedDefs::PostView &post, const bool thumb);
QStringList copyImagesFromRecord(const AppBskyEmbedRecord::ViewRecord &record, const bool thumb);

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
