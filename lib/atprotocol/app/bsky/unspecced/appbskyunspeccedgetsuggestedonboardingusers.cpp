#include "appbskyunspeccedgetsuggestedonboardingusers.h"
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyUnspeccedGetSuggestedOnboardingUsers::AppBskyUnspeccedGetSuggestedOnboardingUsers(
        QObject *parent)
    : AppBskyGraphGetFollows { parent }
{
    m_listKey = QStringLiteral("actors");
}

void AppBskyUnspeccedGetSuggestedOnboardingUsers::getSuggestedOnboardingUsers(
        const QString &category, const int limit)
{
    QUrlQuery url_query;
    if (!category.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("category"), category);
    }
    if (limit > 0) {
        url_query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }

    get(QStringLiteral("xrpc/app.bsky.unspecced.getSuggestedOnboardingUsers"), url_query);
}

}
