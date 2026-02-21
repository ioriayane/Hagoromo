#include "appbskyunspeccedgetonboardingsuggestedusersskeleton.h"
#include "atprotocol/lexicons_func.h"
#include "atprotocol/lexicons_func_unknown.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyUnspeccedGetOnboardingSuggestedUsersSkeleton::
        AppBskyUnspeccedGetOnboardingSuggestedUsersSkeleton(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyUnspeccedGetOnboardingSuggestedUsersSkeleton::getOnboardingSuggestedUsersSkeleton(
        const QString &viewer, const QString &category, const int limit)
{
    QUrlQuery url_query;
    if (!viewer.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("viewer"), viewer);
    }
    if (!category.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("category"), category);
    }
    if (limit > 0) {
        url_query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }

    get(QStringLiteral("xrpc/app.bsky.unspecced.getOnboardingSuggestedUsersSkeleton"), url_query);
}

const QStringList &AppBskyUnspeccedGetOnboardingSuggestedUsersSkeleton::didsList() const
{
    return m_didsList;
}

const QString &AppBskyUnspeccedGetOnboardingSuggestedUsersSkeleton::recId() const
{
    return m_recId;
}

bool AppBskyUnspeccedGetOnboardingSuggestedUsersSkeleton::parseJson(bool success,
                                                                    const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::LexiconsTypeUnknown::copyStringList(
                json_doc.object().value("dids").toArray(), m_didsList);
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("recId"), m_recId);
    }

    return success;
}

}
