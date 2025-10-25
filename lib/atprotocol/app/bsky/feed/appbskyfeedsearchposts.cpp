#include "appbskyfeedsearchposts.h"
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyFeedSearchPosts::AppBskyFeedSearchPosts(QObject *parent) : AppBskyFeedGetPosts { parent }
{
    m_listKey = QStringLiteral("posts");
}

void AppBskyFeedSearchPosts::searchPosts(const QString &q, const QString &sort,
                                         const QString &since, const QString &until,
                                         const QString &mentions, const QString &author,
                                         const QString &lang, const QString &domain,
                                         const QString &url, const QList<QString> &tag,
                                         const qint64 limit, const QString &cursor)
{
    QUrlQuery url_query;
    if (!q.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("q"), q);
    }
    if (!sort.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("sort"), sort);
    }
    if (!since.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("since"), since);
    }
    if (!until.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("until"), until);
    }
    if (!mentions.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("mentions"), mentions);
    }
    if (!author.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("author"), author);
    }
    if (!lang.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("lang"), lang);
    }
    if (!domain.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("domain"), domain);
    }
    if (!url.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("url"), url);
    }
    for (const auto &value : tag) {
        url_query.addQueryItem(QStringLiteral("tag"), value);
    }
    if (limit > 0) {
        url_query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!cursor.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    get(QStringLiteral("xrpc/app.bsky.feed.searchPosts"), url_query);
}

}
