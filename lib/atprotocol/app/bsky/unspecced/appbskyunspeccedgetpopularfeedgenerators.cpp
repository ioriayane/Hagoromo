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

void AppBskyUnspeccedGetPopularFeedGenerators::getPopularFeedGenerators()
{
    QUrlQuery query;

    get(QStringLiteral("xrpc/app.bsky.unspecced.getPopularFeedGenerators"), query);
}

const QList<AtProtocolType::AppBskyFeedDefs::GeneratorView> *
AppBskyUnspeccedGetPopularFeedGenerators::generatorViewList() const
{
    return &m_generatorViewList;
}

void AppBskyUnspeccedGetPopularFeedGenerators::parseJson(const QString reply_json)
{
    bool success = false;

    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
    } else if (!json_doc.object().contains("feeds")) {
    } else {
        for (const auto &value : json_doc.object().value("feeds").toArray()) {
            GeneratorView generator;
            copyGeneratorView(value.toObject(), generator);
            m_generatorViewList.append(generator);
        }
        success = true;
    }

    emit finished(success);
}

}
