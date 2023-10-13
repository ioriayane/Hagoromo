#include "appbskygraphgetlists.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyGraphGetLists::AppBskyGraphGetLists(QObject *parent) : AccessAtProtocol { parent } { }

void AppBskyGraphGetLists::getLists(const QString &actor, const int limit, const QString &cursor)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("actor"), actor);
    if (limit > 0) {
        query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!cursor.isEmpty()) {
        query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    get(QStringLiteral("xrpc/app.bsky.graph.getLists"), query);
}

const QList<AtProtocolType::AppBskyGraphDefs::ListView> *AppBskyGraphGetLists::listViewList() const
{
    return &m_listViewList;
}

bool AppBskyGraphGetLists::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        setCursor(json_doc.object().value("cursor").toString());
        for (const auto &obj : json_doc.object().value("lists").toArray()) {
            AtProtocolType::AppBskyGraphDefs::ListView list;
            AtProtocolType::AppBskyGraphDefs::copyListView(obj.toObject(), list);

            m_listViewList.append(list);
        }
    }

    return success;
}

}
