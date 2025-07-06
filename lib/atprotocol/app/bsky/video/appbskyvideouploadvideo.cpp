#include "appbskyvideouploadvideo.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

AppBskyVideoUploadVideo::AppBskyVideoUploadVideo(QObject *parent) : AccessAtProtocol { parent } { }

void AppBskyVideoUploadVideo::uploadVideo(const QString &path)
{
    postWithImage(QStringLiteral("xrpc/app.bsky.video.uploadVideo"), path);
}

const AtProtocolType::AppBskyVideoDefs::JobStatus &AppBskyVideoUploadVideo::jobStatus() const
{
    return m_jobStatus;
}

bool AppBskyVideoUploadVideo::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::AppBskyVideoDefs::copyJobStatus(
                json_doc.object().value("jobStatus").toObject(), m_jobStatus);
        if (m_jobStatus.state.isEmpty()) {
            // lexiconどおりのレスポンスがこないので暫定処理
            AtProtocolType::AppBskyVideoDefs::copyJobStatus(json_doc.object(), m_jobStatus);
        }
    }

    return success;
}

}
