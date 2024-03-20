#include "appbskygraphgetmutes.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyGraphGetMutes::AppBskyGraphGetMutes(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyGraphGetMutes::getMutes(const int limit, const QString &cursor)
{
    QUrlQuery query;
    //    query.addQueryItem(QStringLiteral("name"), name);

    get(QStringLiteral("xrpc/app.bsky.graph.getMutes"), query);
}

bool AppBskyGraphGetMutes::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        //setCursor(json_doc.object().value("cursor").toString());
    }

    return success;
}

}