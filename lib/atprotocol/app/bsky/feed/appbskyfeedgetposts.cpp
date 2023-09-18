#include "appbskyfeedgetposts.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

using namespace AtProtocolType;

namespace AtProtocolInterface {

AppBskyFeedGetPosts::AppBskyFeedGetPosts(QObject *parent) : AccessAtProtocol { parent } { }

void AppBskyFeedGetPosts::getPosts(const QList<QString> &uris)
{
    QUrlQuery query;
    for (const auto &uri : uris) {
        query.addQueryItem(QStringLiteral("uris[]"), uri);
    }

    get(QStringLiteral("xrpc/app.bsky.feed.getPosts"), query);
}

const QList<AppBskyFeedDefs::PostView> *AppBskyFeedGetPosts::postList() const
{
    return &m_postList;
}

bool AppBskyFeedGetPosts::parseJson(bool success, const QString reply_json)
{

    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains("posts")) {
        success = false;
    } else {
        for (const auto &obj : json_doc.object().value("posts").toArray()) {
            AppBskyFeedDefs::PostView post;

            AppBskyFeedDefs::copyPostView(obj.toObject(), post);
            m_postList.append(post);
        }
    }
    return success;
}

} // namespace AtProtocolInterface
