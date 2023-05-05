#ifndef LEXICONS_FUNC_UNKNOWN_CPP
#define LEXICONS_FUNC_UNKNOWN_CPP

#include "lexicons.h"
#include "lexicons_func_unknown.h"

namespace LexiconsTypeUnknown {

void copyUnknown(const QJsonObject &src, QVariant &dest)
{
    if (src.isEmpty())
        return;

    if (src.value("$type").toString() == QStringLiteral("app.bsky.feed.post")) {
        // typeに#以降がないのでmainの定義で参照
        AppBskyFeedPost::Record record;
        record.text = src.value("text").toString();
        record.createdAt = src.value("createdAt").toString();
        dest.setValue<AppBskyFeedPost::Record>(record);
    }
}

}

#endif // LEXICONS_FUNC_UNKNOWN_CPP
