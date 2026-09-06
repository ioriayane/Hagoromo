#ifndef APPBSKYVIDEOFINISHUPLOAD_H
#define APPBSKYVIDEOFINISHUPLOAD_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyVideoFinishUpload : public AccessAtProtocol
{
public:
    explicit AppBskyVideoFinishUpload(QObject *parent = nullptr);

    void finishUpload(const QString &jobId);

    const QString &completedJobId() const;
    const AtProtocolType::AppBskyVideoDefs::JobStatus &jobStatus() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QString m_completedJobId;
    AtProtocolType::AppBskyVideoDefs::JobStatus m_jobStatus;
};

}

#endif // APPBSKYVIDEOFINISHUPLOAD_H
