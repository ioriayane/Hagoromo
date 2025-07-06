#include "appbskyvideouploadvideoex.h"
#include "atprotocol/com/atproto/server/comatprotoservergetserviceauth.h"
#include "atprotocol/app/bsky/video/appbskyvideouploadvideo.h"
#include "atprotocol/app/bsky/video/appbskyvideogetjobstatus.h"

#include <QFileInfo>
#include <QTimer>

using AtProtocolInterface::AppBskyVideoGetJobStatus;
using AtProtocolInterface::AppBskyVideoUploadVideo;
using AtProtocolInterface::ComAtprotoServerGetServiceAuth;

namespace AtProtocolInterface {

AppBskyVideoUploadVideoEx::AppBskyVideoUploadVideoEx(QObject *parent)
    : AppBskyVideoUploadVideo { parent }
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
                    } else if (upload->jobStatus().state == "JOB_STATE_COMPLETED") {
                        qDebug() << "Completed to upload video."
                                 << "\n  =" << upload->jobStatus().state
                                 << "\n  jobId=" << upload->jobStatus().jobId;
                        emit finished(true);
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
            upload->setAccount(account());
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
    auth->getServiceAuth("did:web:video.bsky.app", 0, "app.bsky.video.uploadVideo");
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
                qDebug() << "Completed to process the video."
                         << "\n  =" << status->jobStatus().state
                         << "\n  jobId=" << status->jobStatus().jobId;
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
