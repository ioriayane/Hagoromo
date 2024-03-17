#include "appbskylabelergetservices.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyLabelerGetServices::AppBskyLabelerGetServices(QObject *parent) : AccessAtProtocol { parent }
{
}

const QList<AtProtocolType::AppBskyLabelerDefs::LabelerViewDetailed> *
AppBskyLabelerGetServices::labelerViewDetails()
{
    return &m_labelerViewDetails;
}

void AppBskyLabelerGetServices::getServices(const QList<QString> &dids, const bool detailed)
{
    QUrlQuery query;
    for (const auto &did : dids) {
        query.addQueryItem(QStringLiteral("dids"), did);
    }
    if (detailed) {
        query.addQueryItem(QStringLiteral("detailed"), "true");
    }

    get(QStringLiteral("xrpc/app.bsky.labeler.getServices"), query);
}

bool AppBskyLabelerGetServices::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        for (const auto &obj : json_doc.object().value("views").toArray()) {
            AtProtocolType::AppBskyLabelerDefs::LabelerViewDetailed labeler;
            AtProtocolType::AppBskyLabelerDefs::copyLabelerViewDetailed(obj.toObject(), labeler);
            m_labelerViewDetails.append(labeler);
        }
    }

    return success;
}

}
