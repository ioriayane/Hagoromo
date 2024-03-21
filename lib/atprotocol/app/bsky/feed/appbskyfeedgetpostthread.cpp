#include "appbskyfeedgetpostthread.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyFeedGetPostThread::AppBskyFeedGetPostThread(QObject *parent)
    : AccessAtProtocol { parent } { }

void AppBskyFeedGetPostThread::getPostThread(const QString &uri)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("uri"), uri);

    get(QStringLiteral("xrpc/app.bsky.feed.getPostThread"), query);
}

const AtProtocolType::AppBskyFeedDefs::ThreadViewPost *
AppBskyFeedGetPostThread::threadViewPost() const
{
    return &m_threadViewPost;
}

bool AppBskyFeedGetPostThread::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains("thread")) {
        success = false;
    } else {
        qDebug().noquote().nospace() << reply_json;
        AtProtocolType::AppBskyFeedDefs::copyThreadViewPost(
                json_doc.object().value("thread").toObject(), m_threadViewPost);
    }

    return success;
}

}
