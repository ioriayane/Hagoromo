#include "appbskyfeedgetactorfeeds.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

using namespace AtProtocolType::AppBskyFeedDefs;

namespace AtProtocolInterface {

AppBskyFeedGetActorFeeds::AppBskyFeedGetActorFeeds(QObject *parent)
    : AccessAtProtocol { parent } { }

void AppBskyFeedGetActorFeeds::getActorFeeds(const QString &actor, const int limit,
                                             const QString &cursor)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("actor"), actor);
    if (limit > 0) {
        query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!cursor.isEmpty()) {
        query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    get(QStringLiteral("xrpc/app.bsky.feed.getActorFeeds"), query);
}

const QList<AtProtocolType::AppBskyFeedDefs::GeneratorView> *
AppBskyFeedGetActorFeeds::generatorViewList() const
{
    return &m_generatorViewList;
}

void AppBskyFeedGetActorFeeds::parseJson(bool success, const QString reply_json)
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

    emit finished(success);
}

}
