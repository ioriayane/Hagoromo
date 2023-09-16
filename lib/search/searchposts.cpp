#include "searchposts.h"
#include "search/search_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>

namespace SearchInterface {

SearchPosts::SearchPosts(QObject *parent) : AtProtocolInterface::AccessAtProtocol { parent } { }

void SearchPosts::search(const QString &text)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("q"), text);

    get(QStringLiteral("search/posts"), query, false);
}

const QList<SearchType::ViewPost> *SearchPosts::viewPostList() const
{
    return &m_viewPostList;
}

bool SearchPosts::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else if (json_doc.isArray()) {
        for (const auto &value : json_doc.array()) {
            SearchType::ViewPost view_post;
            SearchType::copyViewPost(value.toObject(), view_post);
            m_viewPostList.append(view_post);
        }
    }

    return success;
}

}
