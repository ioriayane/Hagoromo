#include "appbskyvideogetjobstatus.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyVideoGetJobStatus::AppBskyVideoGetJobStatus(QObject *parent)
    : AccessAtProtocol { parent } { }

void AppBskyVideoGetJobStatus::getJobStatus(const QString &jobId)
{
    QUrlQuery url_query;
    if (!jobId.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("jobId"), jobId);
    }

    get(QStringLiteral("xrpc/app.bsky.video.getJobStatus"), url_query);
}

const AtProtocolType::AppBskyVideoDefs::JobStatus &AppBskyVideoGetJobStatus::jobStatus() const
{
    return m_jobStatus;
}

bool AppBskyVideoGetJobStatus::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::AppBskyVideoDefs::copyJobStatus(
                json_doc.object().value("jobStatus").toObject(), m_jobStatus);
    }

    return success;
}

}
