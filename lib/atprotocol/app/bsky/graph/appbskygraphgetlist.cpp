#include "appbskygraphgetlist.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyGraphGetList::AppBskyGraphGetList(QObject *parent) : AccessAtProtocol { parent } { }

bool AppBskyGraphGetList::getList(const QString &list, const int limit, const QString &cursor)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("list"), list);
    if (limit > 0) {
        query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!cursor.isEmpty()) {
        query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    return get(QStringLiteral("xrpc/app.bsky.graph.getList"), query);
}

const AtProtocolType::AppBskyGraphDefs::ListView *AppBskyGraphGetList::listView() const
{
    return &m_listView;
}

const QList<AtProtocolType::AppBskyGraphDefs::ListItemView> *
AppBskyGraphGetList::listItemViewList() const
{
    return &m_listItemViewList;
}

bool AppBskyGraphGetList::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        setCursor(json_doc.object().value("cursor").toString());
        AtProtocolType::AppBskyGraphDefs::copyListView(json_doc.object().value("list").toObject(),
                                                       m_listView);

        for (const auto &obj : json_doc.object().value("items").toArray()) {
            AtProtocolType::AppBskyGraphDefs::ListItemView item;
            AtProtocolType::AppBskyGraphDefs::copyListItemView(obj.toObject(), item);

            m_listItemViewList.append(item);
        }
    }

    return success;
}

}
