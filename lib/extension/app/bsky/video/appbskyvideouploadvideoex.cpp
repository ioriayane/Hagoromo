#include "appbskyvideouploadvideoex.h"
#include "atprotocol/com/atproto/server/comatprotoservergetserviceauth.h"
#include "atprotocol/app/bsky/video/appbskyvideouploadvideo.h"
#include "atprotocol/app/bsky/video/appbskyvideogetjobstatus.h"

#include <QFileInfo>
#include <QMimeDatabase>
#include <QTimer>

using AtProtocolInterface::AppBskyVideoGetJobStatus;
using AtProtocolInterface::AppBskyVideoUploadVideo;
using AtProtocolInterface::ComAtprotoServerGetServiceAuth;

namespace AtProtocolInterface {

AppBskyVideoUploadVideoEx::AppBskyVideoUploadVideoEx(QObject *parent)
    : AppBskyVideoUploadVideo { parent }, m_endpoint("https://video.bsky.app")
{
}

void AppBskyVideoUploadVideoEx::uploadVideo(const QString &path)
{
    QFileInfo info(path);
    if (!info.exists()) {
        emit finished(false);
        return;
    }
    getServiceAuth([=](const QString &token) {
        if (token.isEmpty()) {
            qDebug().noquote() << "Can't get service token.";
            emit finished(false);
        } else {
            qDebug() << "Upload process";

            AppBskyVideoUploadVideo *upload = new AppBskyVideoUploadVideo(this);
            connect(upload, &AppBskyVideoUploadVideo::finished, [=](bool success) {
                if (success) {
                    if (upload->jobStatus().state == "JOB_STATE_FAILED") {
                        qDebug() << "Failed to upload video."
                                 << "\n  =" << upload->jobStatus().state
                                 << "\n  jobId=" << upload->jobStatus().jobId;
                        emit finished(false);
                    } else {
                        qDebug() << "Check job status."
                                 << "\n  =" << upload->jobStatus().state
                                 << "\n  jobId=" << upload->jobStatus().jobId;

                        m_jobId = upload->jobStatus().jobId;
                        QTimer::singleShot(0, this, &AppBskyVideoUploadVideoEx::checkJobStatus);
                    }
                } else {
                    qDebug().noquote() << "Failed to upload video.";
                    emit finished(false);
                }
                upload->deleteLater();
            });
            AtProtocolInterface::AccountData a = account();
            a.accessJwt = token;
            a.service_endpoint = m_endpoint;
            upload->setAccount(a);
            upload->uploadVideo(path);
        }
    });
}

void AppBskyVideoUploadVideoEx::getServiceAuth(std::function<void(const QString &)> callback)
{
    ComAtprotoServerGetServiceAuth *auth = new ComAtprotoServerGetServiceAuth(this);
    connect(auth, &ComAtprotoServerGetServiceAuth::finished, [=](bool success) {
        QString token;
        if (success) {
            token = auth->token();
        }
        callback(token);
        auth->deleteLater();
    });
    auth->setAccount(account());
    auth->getServiceAuth("did:web:video.bsky.app", 0, "com.atproto.repo.uploadBlob");
}

void AppBskyVideoUploadVideoEx::setEndpoint(const QString &newEndpoint)
{
    if (newEndpoint.isEmpty())
        return;
    m_endpoint = newEndpoint;
}

QString AppBskyVideoUploadVideoEx::cid() const
{
    return m_cid;
}

qint64 AppBskyVideoUploadVideoEx::size() const
{
    return m_size;
}

QString AppBskyVideoUploadVideoEx::mimeType() const
{
    return m_mimeType;
}

QSize AppBskyVideoUploadVideoEx::aspectRatio() const
{
    return m_aspectRatio;
}

void AppBskyVideoUploadVideoEx::checkJobStatus()
{
    if (m_jobId.isEmpty()) {
        emit finished(false);
        return;
    }

    AppBskyVideoGetJobStatus *status = new AppBskyVideoGetJobStatus(this);
    connect(status, &AppBskyVideoGetJobStatus::finished, [=](bool success) {
        if (success) {
            //
            if (status->jobStatus().state == "JOB_STATE_FAILED") {
                qDebug() << "Failed to process the video."
                         << "\n  =" << status->jobStatus().state
                         << "\n  jobId=" << status->jobStatus().jobId;
                emit finished(false);
            } else if (status->jobStatus().state == "JOB_STATE_COMPLETED") {
                m_cid = status->jobStatus().blob.cid;
                m_mimeType = status->jobStatus().blob.mimeType;
                m_size = status->jobStatus().blob.size;
                qDebug() << "Completed to process the video."
                         << "\n  =" << status->jobStatus().state
                         << "\n  jobId=" << status->jobStatus().jobId << "\n  cid=" << m_cid
                         << "\n mimeType=" << m_mimeType << "\n  size=" << m_size;
                emit finished(true);
            } else {
                qDebug() << "Check job status."
                         << "\n  =" << status->jobStatus().state
                         << "\n  jobId=" << status->jobStatus().jobId;
                QTimer::singleShot(2000, this, &AppBskyVideoUploadVideoEx::checkJobStatus);
            }
        } else {
            qDebug().noquote() << "Failed to checked status.";
            emit finished(false);
        }
        status->deleteLater();
    });
    status->setAccount(account());
    status->getJobStatus(m_jobId);
}
}
