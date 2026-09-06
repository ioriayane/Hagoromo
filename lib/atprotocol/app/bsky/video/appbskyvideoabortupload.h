#ifndef APPBSKYVIDEOABORTUPLOAD_H
#define APPBSKYVIDEOABORTUPLOAD_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyVideoAbortUpload : public AccessAtProtocol
{
public:
    explicit AppBskyVideoAbortUpload(QObject *parent = nullptr);

    void abortUpload(const QString &jobId);

    const QString &state() const;
    const QString &completedJobId() const;
    const QString &failureReason() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QString m_state;
    QString m_completedJobId;
    QString m_failureReason;
};

}

#endif // APPBSKYVIDEOABORTUPLOAD_H
