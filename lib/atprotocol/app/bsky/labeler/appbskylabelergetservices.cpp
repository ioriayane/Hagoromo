#include "appbskylabelergetservices.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyLabelerGetServices::AppBskyLabelerGetServices(QObject *parent) : AccessAtProtocol { parent }
{
}

void AppBskyLabelerGetServices::getServices(const QList<QString> &dids, const bool detailed)
{
    QUrlQuery url_query;
    for (const auto &value : dids) {
        url_query.addQueryItem(QStringLiteral("dids"), value);
    }
    if (detailed) {
        url_query.addQueryItem(QStringLiteral("detailed"), "true");
    }

    get(QStringLiteral("xrpc/app.bsky.labeler.getServices"), url_query);
}

const QList<AtProtocolType::AppBskyLabelerDefs::LabelerView> &
AppBskyLabelerGetServices::viewsLabelerViewList() const
{
    return m_viewsLabelerViewList;
}

const QList<AtProtocolType::AppBskyLabelerDefs::LabelerViewDetailed> &
AppBskyLabelerGetServices::viewsLabelerViewDetailedList() const
{
    return m_viewsLabelerViewDetailedList;
}

bool AppBskyLabelerGetServices::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains("views")) {
        success = false;
    } else {
        QString type;
        for (const auto &value : json_doc.object().value("views").toArray()) {
            type = value.toObject().value("$type").toString();
            if (type == QStringLiteral("app.bsky.labeler.defs#labelerView")) {
                AtProtocolType::AppBskyLabelerDefs::LabelerView data;
                AtProtocolType::AppBskyLabelerDefs::copyLabelerView(value.toObject(), data);
                m_viewsLabelerViewList.append(data);
            } else if (type == QStringLiteral("app.bsky.labeler.defs#labelerViewDetailed")) {
                AtProtocolType::AppBskyLabelerDefs::LabelerViewDetailed data;
                AtProtocolType::AppBskyLabelerDefs::copyLabelerViewDetailed(value.toObject(), data);
                m_viewsLabelerViewDetailedList.append(data);
            }
        }
    }

    return success;
}

}
