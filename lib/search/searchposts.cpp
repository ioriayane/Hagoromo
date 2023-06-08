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

QList<SearchType::ViewPost> *SearchPosts::viewPostList()
{
    return &m_viewPostList;
}

void SearchPosts::parseJson(const QString reply_json)
{
    bool success = false;

    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
    } else if (json_doc.isArray()) {
        for (const auto &value : json_doc.array()) {
            SearchType::ViewPost view_post;
            SearchType::copyViewPost(value.toObject(), view_post);
            m_viewPostList.append(view_post);
        }
        success = true;
    }

    emit finished(success);
}

}
