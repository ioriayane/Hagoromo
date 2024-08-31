#ifndef APPBSKYVIDEOGETJOBSTATUS_H
#define APPBSKYVIDEOGETJOBSTATUS_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyVideoGetJobStatus : public AccessAtProtocol
{
public:
    explicit AppBskyVideoGetJobStatus(QObject *parent = nullptr);

    void getJobStatus(const QString &jobId);

    const AtProtocolType::AppBskyVideoDefs::JobStatus &jobStatus() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::AppBskyVideoDefs::JobStatus m_jobStatus;
};

}

#endif // APPBSKYVIDEOGETJOBSTATUS_H
