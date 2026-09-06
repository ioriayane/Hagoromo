#include "appbskyfeedsearchpostsv2.h"
#include "atprotocol/lexicons_func.h"
#include "atprotocol/lexicons_func_unknown.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyFeedSearchPostsV2::AppBskyFeedSearchPostsV2(QObject *parent) : AppBskyFeedGetPosts { parent }
{
}

void AppBskyFeedSearchPostsV2::searchPostsV2(
        const QString &cursor, const int limit, const QString &query, const QString &sort,
        const QList<QString> &authors, const QList<QString> &mentions,
        const QList<QString> &domains, const QList<QString> &urls,
        const QList<QString> &embeddedAtUris, const QList<QString> &hashtags,
        const QList<QString> &excludeAuthors, const QList<QString> &excludeMentions,
        const QList<QString> &excludeDomains, const QList<QString> &excludeUrls,
        const QList<QString> &excludeEmbeddedAtUris, const QList<QString> &excludeHashtags,
        const QString &since, const QString &until, const bool allTime,
        const QList<QString> &languages, const QList<QString> &excludeLanguages,
        const bool hasMedia, const bool hasVideo, const QString &replyParentUri,
        const QString &threadRootUri, const bool excludeReplies, const bool repliesOnly,
        const bool following, const QString &queryLanguage)
{
    QUrlQuery url_query;
    if (!cursor.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("cursor"), cursor);
    }
    if (limit > 0) {
        url_query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!query.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("query"), query);
    }
    if (!sort.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("sort"), sort);
    }
    for (const auto &value : authors) {
        url_query.addQueryItem(QStringLiteral("authors"), value);
    }
    for (const auto &value : mentions) {
        url_query.addQueryItem(QStringLiteral("mentions"), value);
    }
    for (const auto &value : domains) {
        url_query.addQueryItem(QStringLiteral("domains"), value);
    }
    for (const auto &value : urls) {
        url_query.addQueryItem(QStringLiteral("urls"), value);
    }
    for (const auto &value : embeddedAtUris) {
        url_query.addQueryItem(QStringLiteral("embeddedAtUris"), value);
    }
    for (const auto &value : hashtags) {
        url_query.addQueryItem(QStringLiteral("hashtags"), value);
    }
    for (const auto &value : excludeAuthors) {
        url_query.addQueryItem(QStringLiteral("excludeAuthors"), value);
    }
    for (const auto &value : excludeMentions) {
        url_query.addQueryItem(QStringLiteral("excludeMentions"), value);
    }
    for (const auto &value : excludeDomains) {
        url_query.addQueryItem(QStringLiteral("excludeDomains"), value);
    }
    for (const auto &value : excludeUrls) {
        url_query.addQueryItem(QStringLiteral("excludeUrls"), value);
    }
    for (const auto &value : excludeEmbeddedAtUris) {
        url_query.addQueryItem(QStringLiteral("excludeEmbeddedAtUris"), value);
    }
    for (const auto &value : excludeHashtags) {
        url_query.addQueryItem(QStringLiteral("excludeHashtags"), value);
    }
    if (!since.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("since"), since);
    }
    if (!until.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("until"), until);
    }
    if (allTime) {
        url_query.addQueryItem(QStringLiteral("allTime"), "true");
    }
    for (const auto &value : languages) {
        url_query.addQueryItem(QStringLiteral("languages"), value);
    }
    for (const auto &value : excludeLanguages) {
        url_query.addQueryItem(QStringLiteral("excludeLanguages"), value);
    }
    if (hasMedia) {
        url_query.addQueryItem(QStringLiteral("hasMedia"), "true");
    }
    if (hasVideo) {
        url_query.addQueryItem(QStringLiteral("hasVideo"), "true");
    }
    if (!replyParentUri.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("replyParentUri"), replyParentUri);
    }
    if (!threadRootUri.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("threadRootUri"), threadRootUri);
    }
    if (excludeReplies) {
        url_query.addQueryItem(QStringLiteral("excludeReplies"), "true");
    }
    if (repliesOnly) {
        url_query.addQueryItem(QStringLiteral("repliesOnly"), "true");
    }
    if (following) {
        url_query.addQueryItem(QStringLiteral("following"), "true");
    }
    if (!queryLanguage.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("queryLanguage"), queryLanguage);
    }

    get(QStringLiteral("xrpc/app.bsky.feed.searchPostsV2"), url_query);
}

const int &AppBskyFeedSearchPostsV2::hitsTotal() const
{
    return m_hitsTotal;
}

const QList<AtProtocolType::AppBskyFeedDefs::PostView> &AppBskyFeedSearchPostsV2::postsList() const
{
    return m_postsList;
}

const QStringList &AppBskyFeedSearchPostsV2::detectedQueryLanguagesList() const
{
    return m_detectedQueryLanguagesList;
}

bool AppBskyFeedSearchPostsV2::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains("posts")) {
        success = false;
    } else {
        setCursor(json_doc.object().value("cursor").toString());
        AtProtocolType::LexiconsTypeUnknown::copyInt(json_doc.object().value("hitsTotal"),
                                                     m_hitsTotal);
        for (const auto &value : json_doc.object().value("posts").toArray()) {
            AtProtocolType::AppBskyFeedDefs::PostView data;
            AtProtocolType::AppBskyFeedDefs::copyPostView(value.toObject(), data);
            m_postsList.append(data);
        }
        AtProtocolType::LexiconsTypeUnknown::copyStringList(
                json_doc.object().value("detectedQueryLanguages").toArray(),
                m_detectedQueryLanguagesList);
    }

    return success;
}

}
