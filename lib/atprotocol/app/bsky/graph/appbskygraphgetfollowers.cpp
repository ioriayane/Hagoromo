#include "appbskygraphgetfollowers.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyGraphGetFollowers::AppBskyGraphGetFollowers(QObject *parent)
    : AppBskyGraphGetFollows { parent }
{
    m_listKey = QStringLiteral("followers");
}

void AppBskyGraphGetFollowers::getFollowers(const QString &actor, const int limit,
                                            const QString &cursor)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("actor"), actor);

    get(QStringLiteral("xrpc/app.bsky.graph.getFollowers"), query);
}

// void AppBskyGraphGetFollowers::parseJson(const QString reply_json)
//{
//    bool success = false;

//    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
//    if (json_doc.isEmpty()) { }

//    emit finished(success);
//}

}
