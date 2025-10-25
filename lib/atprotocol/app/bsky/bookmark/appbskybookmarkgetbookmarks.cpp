#include "appbskybookmarkgetbookmarks.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyBookmarkGetBookmarks::AppBskyBookmarkGetBookmarks(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyBookmarkGetBookmarks::getBookmarks(const qint64 limit, const QString &cursor)
{
    QUrlQuery url_query;
    if (limit > 0) {
        url_query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!cursor.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    get(QStringLiteral("xrpc/app.bsky.bookmark.getBookmarks"), url_query);
}

const QList<AtProtocolType::AppBskyBookmarkDefs::BookmarkView> &
AppBskyBookmarkGetBookmarks::bookmarksList() const
{
    return m_bookmarksList;
}

bool AppBskyBookmarkGetBookmarks::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains("bookmarks")) {
        success = false;
    } else {
        setCursor(json_doc.object().value("cursor").toString());
        for (const auto &value : json_doc.object().value("bookmarks").toArray()) {
            AtProtocolType::AppBskyBookmarkDefs::BookmarkView data;
            AtProtocolType::AppBskyBookmarkDefs::copyBookmarkView(value.toObject(), data);
            m_bookmarksList.append(data);
        }
    }

    return success;
}

}
