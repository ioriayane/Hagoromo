#include "appbskyunspeccedgetonboardingsuggestedstarterpacksskeleton.h"
#include "atprotocol/lexicons_func.h"
#include "atprotocol/lexicons_func_unknown.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyUnspeccedGetOnboardingSuggestedStarterPacksSkeleton::
        AppBskyUnspeccedGetOnboardingSuggestedStarterPacksSkeleton(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyUnspeccedGetOnboardingSuggestedStarterPacksSkeleton::
        getOnboardingSuggestedStarterPacksSkeleton(const QString &viewer, const int limit)
{
    QUrlQuery url_query;
    if (!viewer.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("viewer"), viewer);
    }
    if (limit > 0) {
        url_query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }

    get(QStringLiteral("xrpc/app.bsky.unspecced.getOnboardingSuggestedStarterPacksSkeleton"),
        url_query);
}

const QStringList &
AppBskyUnspeccedGetOnboardingSuggestedStarterPacksSkeleton::starterPacksList() const
{
    return m_starterPacksList;
}

bool AppBskyUnspeccedGetOnboardingSuggestedStarterPacksSkeleton::parseJson(bool success,
                                                                           const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::LexiconsTypeUnknown::copyStringList(
                json_doc.object().value("starterPacks").toArray(), m_starterPacksList);
    }

    return success;
}

}
