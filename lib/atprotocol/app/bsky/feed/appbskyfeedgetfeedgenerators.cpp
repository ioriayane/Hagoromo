#include "appbskyfeedgetfeedgenerators.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyFeedGetFeedGenerators::AppBskyFeedGetFeedGenerators(QObject *parent)
    : AccessAtProtocol { parent }, m_listKey("feeds")
{
}

void AppBskyFeedGetFeedGenerators::getFeedGenerators(const QList<QString> &feeds)
{
    QUrlQuery url_query;
    for (const auto &value : feeds) {
        url_query.addQueryItem(QStringLiteral("feeds"), value);
    }

    get(QStringLiteral("xrpc/app.bsky.feed.getFeedGenerators"), url_query);
}

const QList<AtProtocolType::AppBskyFeedDefs::GeneratorView> &
AppBskyFeedGetFeedGenerators::generatorViewList() const
{
    return m_generatorViewList;
}

bool AppBskyFeedGetFeedGenerators::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains(m_listKey)) {
        success = false;
    } else {
        for (const auto &value : json_doc.object().value(m_listKey).toArray()) {
            AtProtocolType::AppBskyFeedDefs::GeneratorView data;
            AtProtocolType::AppBskyFeedDefs::copyGeneratorView(value.toObject(), data);
            m_generatorViewList.append(data);
        }
    }

    return success;
}

}
