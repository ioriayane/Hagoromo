#include "appbskyvideogetuploadstatus.h"
#include "atprotocol/lexicons_func.h"
#include "atprotocol/lexicons_func_unknown.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyVideoGetUploadStatus::AppBskyVideoGetUploadStatus(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyVideoGetUploadStatus::getUploadStatus(const QString &jobId)
{
    QUrlQuery url_query;
    if (!jobId.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("jobId"), jobId);
    }

    get(QStringLiteral("xrpc/app.bsky.video.getUploadStatus"), url_query);
}

const QString &AppBskyVideoGetUploadStatus::jobId() const
{
    return m_jobId;
}

const int &AppBskyVideoGetUploadStatus::partSizeBytes() const
{
    return m_partSizeBytes;
}

const int &AppBskyVideoGetUploadStatus::partCount() const
{
    return m_partCount;
}

const QString &AppBskyVideoGetUploadStatus::expiresAt() const
{
    return m_expiresAt;
}

const QString &AppBskyVideoGetUploadStatus::state() const
{
    return m_state;
}

const QString &AppBskyVideoGetUploadStatus::completedJobId() const
{
    return m_completedJobId;
}

const AtProtocolType::AppBskyVideoDefs::JobStatus &AppBskyVideoGetUploadStatus::jobStatus() const
{
    return m_jobStatus;
}

const QString &AppBskyVideoGetUploadStatus::failureReason() const
{
    return m_failureReason;
}

bool AppBskyVideoGetUploadStatus::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("jobId"), m_jobId);
        AtProtocolType::LexiconsTypeUnknown::copyInt(json_doc.object().value("partSizeBytes"),
                                                     m_partSizeBytes);
        AtProtocolType::LexiconsTypeUnknown::copyInt(json_doc.object().value("partCount"),
                                                     m_partCount);
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("expiresAt"),
                                                        m_expiresAt);
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("state"), m_state);
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("completedJobId"),
                                                        m_completedJobId);
        AtProtocolType::AppBskyVideoDefs::copyJobStatus(
                json_doc.object().value("jobStatus").toObject(), m_jobStatus);
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("failureReason"),
                                                        m_failureReason);
    }

    return success;
}

}
