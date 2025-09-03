#include "appbskybookmarkcreatebookmark.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

AppBskyBookmarkCreateBookmark::AppBskyBookmarkCreateBookmark(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyBookmarkCreateBookmark::createBookmark(const QString &uri, const QString &cid)
{
    QJsonObject json_obj;
    if (!uri.isEmpty()) {
        json_obj.insert(QStringLiteral("uri"), uri);
    }
    if (!cid.isEmpty()) {
        json_obj.insert(QStringLiteral("cid"), cid);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/app.bsky.bookmark.createBookmark"),
         json_doc.toJson(QJsonDocument::Compact));
}

bool AppBskyBookmarkCreateBookmark::parseJson(bool success, const QString reply_json)
{
    Q_UNUSED(reply_json)
    return success;
}

}
