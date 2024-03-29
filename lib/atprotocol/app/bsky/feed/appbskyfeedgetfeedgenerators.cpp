#include "appbskyfeedgetfeedgenerators.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>

using namespace AtProtocolType::AppBskyFeedDefs;

namespace AtProtocolInterface {

AppBskyFeedGetFeedGenerators::AppBskyFeedGetFeedGenerators(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyFeedGetFeedGenerators::getFeedGenerators(const QList<QString> &feeds)
{
    QUrlQuery query;
    for (const auto &feed : feeds) {
        query.addQueryItem(QStringLiteral("feeds[]"), feed);
    }

    get(QStringLiteral("xrpc/app.bsky.feed.getFeedGenerators"), query);
}

const QList<AtProtocolType::AppBskyFeedDefs::GeneratorView> *
AppBskyFeedGetFeedGenerators::generatorViewList() const
{
    return &m_generatorViewList;
}

bool AppBskyFeedGetFeedGenerators::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains("feeds")) {
        success = false;
    } else {
        for (const auto &value : json_doc.object().value("feeds").toArray()) {
            GeneratorView generator;
            copyGeneratorView(value.toObject(), generator);
            m_generatorViewList.append(generator);
        }
    }

    return success;
}

}
