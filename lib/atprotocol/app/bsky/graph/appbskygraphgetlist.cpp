#include "appbskygraphgetlist.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyGraphGetList::AppBskyGraphGetList(QObject *parent) : AccessAtProtocol { parent } { }

void AppBskyGraphGetList::getList(const QString &list, const int limit, const QString &cursor)
{
    QUrlQuery url_query;
    if (!list.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("list"), list);
    }
    if (limit > 0) {
        url_query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!cursor.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    get(QStringLiteral("xrpc/app.bsky.graph.getList"), url_query);
}

const AtProtocolType::AppBskyGraphDefs::ListView &AppBskyGraphGetList::list() const
{
    return m_list;
}

const QList<AtProtocolType::AppBskyGraphDefs::ListItemView> &AppBskyGraphGetList::itemsList() const
{
    return m_itemsList;
}

bool AppBskyGraphGetList::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains("items")) {
        success = false;
    } else {
        setCursor(json_doc.object().value("cursor").toString());
        AtProtocolType::AppBskyGraphDefs::copyListView(json_doc.object().value("list").toObject(),
                                                       m_list);
        for (const auto &value : json_doc.object().value("items").toArray()) {
            AtProtocolType::AppBskyGraphDefs::ListItemView data;
            AtProtocolType::AppBskyGraphDefs::copyListItemView(value.toObject(), data);
            m_itemsList.append(data);
        }
    }

    return success;
}

}
