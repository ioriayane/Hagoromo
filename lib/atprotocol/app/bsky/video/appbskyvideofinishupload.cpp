#include "appbskyvideofinishupload.h"
#include "atprotocol/lexicons_func.h"
#include "atprotocol/lexicons_func_unknown.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

AppBskyVideoFinishUpload::AppBskyVideoFinishUpload(QObject *parent)
    : AccessAtProtocol { parent } { }

void AppBskyVideoFinishUpload::finishUpload(const QString &jobId)
{
    QJsonObject json_obj;
    if (!jobId.isEmpty()) {
        json_obj.insert(QStringLiteral("jobId"), jobId);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/app.bsky.video.finishUpload"),
         json_doc.toJson(QJsonDocument::Compact));
}

const QString &AppBskyVideoFinishUpload::completedJobId() const
{
    return m_completedJobId;
}

const AtProtocolType::AppBskyVideoDefs::JobStatus &AppBskyVideoFinishUpload::jobStatus() const
{
    return m_jobStatus;
}

bool AppBskyVideoFinishUpload::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("completedJobId"),
                                                        m_completedJobId);
        AtProtocolType::AppBskyVideoDefs::copyJobStatus(
                json_doc.object().value("jobStatus").toObject(), m_jobStatus);
    }

    return success;
}

}
