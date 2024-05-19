#include "appbskyfeedgetposts.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyFeedGetPosts::AppBskyFeedGetPosts(QObject *parent)
    : AccessAtProtocol { parent }, m_listKey("posts")
{
}

void AppBskyFeedGetPosts::getPosts(const QList<QString> &uris)
{
    QUrlQuery url_query;
    for (const auto &value : uris) {
        url_query.addQueryItem(QStringLiteral("uris"), value);
    }

    get(QStringLiteral("xrpc/app.bsky.feed.getPosts"), url_query);
}

const QList<AtProtocolType::AppBskyFeedDefs::PostView> &
AppBskyFeedGetPosts::postsPostViewList() const
{
    return m_postsPostViewList;
}

bool AppBskyFeedGetPosts::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains(m_listKey)) {
        success = false;
    } else {
        for (const auto &value : json_doc.object().value(m_listKey).toArray()) {
            AtProtocolType::AppBskyFeedDefs::PostView data;
            AtProtocolType::AppBskyFeedDefs::copyPostView(value.toObject(), data);
            m_postsPostViewList.append(data);
        }
    }

    return success;
}

}
