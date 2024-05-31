#ifndef LEXICONS_FUNC_UNKNOWN_H
#define LEXICONS_FUNC_UNKNOWN_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QList>
#include <QString>
#include <QVariant>

#include "lexicons.h"
#include "atprotocol/accessatprotocol.h"

#define REG_EXP_URL                                                                                \
    "http[s]?://"                                                                                  \
    "(?:[a-zA-Z]|[0-9]|[$-_@.&+]|[!*\\(\\),]|(?:%[0-9a-fA-F][0-9a-fA-F])|#(?:[a-zA-Z]|[0-9]|[$-_@" \
    ".&+]|[!*\\(\\),]|(?:%[0-9a-fA-F][0-9a-fA-F]))*)+"
#define REG_EXP_MENTION                                                                            \
    "@(?:[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?\\.)+(?:[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-" \
    "zA-Z0-9])?)"
#define REG_EXP_HANDLE                                                                             \
    "(?:[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?\\.)+(?:[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-"  \
    "zA-Z0-9])?)"
#define REG_EXP_MENTION_PART "@[a-zA-Z0-9][a-zA-Z0-9.-]*$"
#define REG_EXP_HASH_TAG                                                                           \
    QStringLiteral("(?:^|"                                                                         \
                   "[ \\f\\n\\r\\t\\v%1%2%3-%4%5%6%7%8%9%10])"                                     \
                   "(?:#[^0-9 \\f\\n\\r\\t\\v%1%2%3-%4%5%6%7%8%9%10]"                              \
                   "[^ \\f\\n\\r\\t\\v%1%2%3-%4%5%6%7%8%9%10]*)"                                   \
                   "(?:$|[ \\f\\n\\r\\t\\v%1%2%3-%4%5%6%7%8%9%10]?)")                              \
            .arg(QChar(0x00a0))                                                                    \
            .arg(QChar(0x1680))                                                                    \
            .arg(QChar(0x2000))                                                                    \
            .arg(QChar(0x200a))                                                                    \
            .arg(QChar(0x2028))                                                                    \
            .arg(QChar(0x2029))                                                                    \
            .arg(QChar(0x202f))                                                                    \
            .arg(QChar(0x205f))                                                                    \
            .arg(QChar(0x3000))                                                                    \
            .arg(QChar(0xfeff))

namespace AtProtocolType {
namespace LexiconsTypeUnknown {

void copyUnknown(const QJsonObject &src, QVariant &dest);
void copyBlob(const QJsonObject &src, Blob &dest);
void copyString(const QJsonValue &src, QString &dest);
void copyStringList(const QJsonArray &src, QStringList &dest);
void copyBool(const QJsonValue &src, bool &dest);
void copyInt(const QJsonValue &src, int &dest);

enum class CopyImageType : int {
    Thumb,
    FullSize,
    Alt,
};

QStringList copyImagesFromPostView(const AppBskyFeedDefs::PostView &post, const CopyImageType type);
QStringList copyImagesFromRecord(const AppBskyEmbedRecord::ViewRecord &record,
                                 const CopyImageType type);
QStringList copyTagsFromFacets(const QList<AppBskyRichtextFacet::Main> &facets);
bool checkPartialMatchLanguage(const QStringList &langs);
QString copyRecordText(const QVariant &value);
QString formatDateTime(const QString &value, const bool is_long = false);

void makeFacets(QObject *parent, AtProtocolInterface::AccountData account, const QString &text,
                std::function<void(const QList<AtProtocolType::AppBskyRichtextFacet::Main> &facets)>
                        callback);
void insertFacetsJson(QJsonObject &parent,
                      const QList<AtProtocolType::AppBskyRichtextFacet::Main> &facets);
QString applyFacetsTo(const QString &text, const QList<AppBskyRichtextFacet::Main> &text_facets);

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
