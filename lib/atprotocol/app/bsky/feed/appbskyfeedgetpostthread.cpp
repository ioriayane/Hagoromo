#include "appbskyfeedgetpostthread.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyFeedGetPostThread::AppBskyFeedGetPostThread(QObject *parent)
    : AccessAtProtocol { parent } { }

void AppBskyFeedGetPostThread::getPostThread(const QString &uri, const int depth,
                                             const int parentHeight)
{
    QUrlQuery url_query;
    if (!uri.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("uri"), uri);
    }
    if (depth > 0) {
        url_query.addQueryItem(QStringLiteral("depth"), QString::number(depth));
    }
    if (parentHeight > 0) {
        url_query.addQueryItem(QStringLiteral("parentHeight"), QString::number(parentHeight));
    }

    get(QStringLiteral("xrpc/app.bsky.feed.getPostThread"), url_query);
}

const AtProtocolType::AppBskyFeedDefs::ThreadViewPost &
AppBskyFeedGetPostThread::threadViewPost() const
{
    return m_threadViewPost;
}

const AtProtocolType::AppBskyFeedDefs::NotFoundPost &AppBskyFeedGetPostThread::notFoundPost() const
{
    return m_notFoundPost;
}

const AtProtocolType::AppBskyFeedDefs::BlockedPost &AppBskyFeedGetPostThread::blockedPost() const
{
    return m_blockedPost;
}

bool AppBskyFeedGetPostThread::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        QString type;
        type = json_doc.object().value("thread").toObject().value("$type").toString();
        if (type == QStringLiteral("app.bsky.feed.defs#threadViewPost")) {
            AtProtocolType::AppBskyFeedDefs::copyThreadViewPost(
                    json_doc.object().value("thread").toObject(), m_threadViewPost);
        } else if (type == QStringLiteral("app.bsky.feed.defs#notFoundPost")) {
            AtProtocolType::AppBskyFeedDefs::copyNotFoundPost(
                    json_doc.object().value("thread").toObject(), m_notFoundPost);
        } else if (type == QStringLiteral("app.bsky.feed.defs#blockedPost")) {
            AtProtocolType::AppBskyFeedDefs::copyBlockedPost(
                    json_doc.object().value("thread").toObject(), m_blockedPost);
        }
    }

    return success;
}

}
