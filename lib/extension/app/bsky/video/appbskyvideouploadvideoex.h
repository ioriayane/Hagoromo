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
};
}
#endif // APPBSKYVIDEOUPLOADVIDEOEX_H
