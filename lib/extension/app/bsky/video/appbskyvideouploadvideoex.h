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

private:
    QString m_jobId;
    void checkJobStatus();
};
}
#endif // APPBSKYVIDEOUPLOADVIDEOEX_H
