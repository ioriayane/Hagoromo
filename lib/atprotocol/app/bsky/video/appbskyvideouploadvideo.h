#ifndef APPBSKYVIDEOUPLOADVIDEO_H
#define APPBSKYVIDEOUPLOADVIDEO_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyVideoUploadVideo : public AccessAtProtocol
{
public:
    explicit AppBskyVideoUploadVideo(QObject *parent = nullptr);

    void uploadVideo();

    const AtProtocolType::AppBskyVideoDefs::JobStatus &jobStatus() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::AppBskyVideoDefs::JobStatus m_jobStatus;
};

}

#endif // APPBSKYVIDEOUPLOADVIDEO_H
