#include "appbskyunspeccedgetpopularfeedgenerators.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>

using namespace AtProtocolType::AppBskyFeedDefs;

namespace AtProtocolInterface {

AppBskyUnspeccedGetPopularFeedGenerators::AppBskyUnspeccedGetPopularFeedGenerators(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyUnspeccedGetPopularFeedGenerators::getPopularFeedGenerators(const int limit,
                                                                        const QString &cursor,
                                                                        const QString &query)
{
    QUrlQuery url_query;
    if (!query.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("query"), query);
    }
    if (!cursor.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    get(QStringLiteral("xrpc/app.bsky.unspecced.getPopularFeedGenerators"), url_query);
}

const QList<AtProtocolType::AppBskyFeedDefs::GeneratorView> *
AppBskyUnspeccedGetPopularFeedGenerators::generatorViewList() const
{
    return &m_generatorViewList;
}

bool AppBskyUnspeccedGetPopularFeedGenerators::parseJson(bool success, const QString reply_json)
{

    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains("feeds")) {
        success = false;
    } else {
        setCursor(json_doc.object().value("cursor").toString());
        for (const auto &value : json_doc.object().value("feeds").toArray()) {
            GeneratorView generator;
            copyGeneratorView(value.toObject(), generator);
            m_generatorViewList.append(generator);
        }
    }

    return success;
}

}
