#ifndef APPBSKYVIDEOGETUPLOADLIMITSEX_H
#define APPBSKYVIDEOGETUPLOADLIMITSEX_H

#include "atprotocol/app/bsky/video/appbskyvideogetuploadlimits.h"

namespace AtProtocolInterface {

class AppBskyVideoGetUploadLimitsEx : public AppBskyVideoGetUploadLimits
{
    Q_OBJECT
public:
    explicit AppBskyVideoGetUploadLimitsEx(QObject *parent = nullptr);

    void canUpload(const QString &path);

    void getServiceAuth(std::function<void(const QString &)> callback);

    void setEndpoint(const QString &newEndpoint);

private:
    QString m_endpoint;
};

}

#endif // APPBSKYVIDEOGETUPLOADLIMITSEX_H
