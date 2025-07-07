#ifndef APPBSKYVIDEOUPLOADVIDEOEX_H
#define APPBSKYVIDEOUPLOADVIDEOEX_H

#include "atprotocol/app/bsky/video/appbskyvideouploadvideo.h"

namespace AtProtocolInterface {

class AppBskyVideoUploadVideoEx : public AppBskyVideoUploadVideo
{
    Q_OBJECT
public:
    explicit AppBskyVideoUploadVideoEx(QObject *parent = nullptr);

    void uploadVideo(const QString &path);

    void getServiceAuth(std::function<void(const QString &)> callback);

    void setEndpoint(const QString &newEndpoint);

    QString cid() const;
    qint64 size() const;
    QString mimeType() const;
    QSize aspectRatio() const;

private:
    QString m_endpoint;
    QString m_jobId;

    QString m_cid;
    qint64 m_size;
    QString m_mimeType;
    QSize m_aspectRatio;

    void checkJobStatus();
};
}
#endif // APPBSKYVIDEOUPLOADVIDEOEX_H
