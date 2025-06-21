#ifndef APPBSKYVIDEOGETUPLOADLIMITSEX_H
#define APPBSKYVIDEOGETUPLOADLIMITSEX_H

#include "atprotocol/app/bsky/video/appbskyvideogetuploadlimits.h"

namespace AtProtocolInterface {

class AppBskyVideoGetUploadLimitsEx : public AppBskyVideoGetUploadLimits
{
    Q_OBJECT
public:
    explicit AppBskyVideoGetUploadLimitsEx(QObject *parent = nullptr);
};

}

#endif // APPBSKYVIDEOGETUPLOADLIMITSEX_H
