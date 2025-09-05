#include "appbskybookmarkdeletebookmark.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

AppBskyBookmarkDeleteBookmark::AppBskyBookmarkDeleteBookmark(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyBookmarkDeleteBookmark::deleteBookmark(const QString &uri)
{
    QJsonObject json_obj;
    if (!uri.isEmpty()) {
        json_obj.insert(QStringLiteral("uri"), uri);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/app.bsky.bookmark.deleteBookmark"),
         json_doc.toJson(QJsonDocument::Compact));
}

bool AppBskyBookmarkDeleteBookmark::parseJson(bool success, const QString reply_json)
{
    Q_UNUSED(reply_json)
    return success;
}

}
