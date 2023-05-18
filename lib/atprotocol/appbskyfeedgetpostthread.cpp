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

AtProtocolType::AppBskyFeedDefs::ThreadViewPost *AppBskyFeedGetPostThread::threadViewPost()
{
    return &m_threadViewPost;
}

void AppBskyFeedGetPostThread::parseJson(const QString reply_json)
{
    bool success = false;

    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        qDebug() << "EMPTY";
    } else if (!json_doc.object().contains("thread")) {
        qDebug() << "Not found thread";
    } else {
        AtProtocolType::AppBskyFeedDefs::copyThreadViewPost(
                json_doc.object().value("thread").toObject(), m_threadViewPost);
        success = true;
    }

    emit finished(success);
}

}
