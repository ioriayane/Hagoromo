#ifndef APPBSKYVIDEOGETUPLOADLIMITS_H
#define APPBSKYVIDEOGETUPLOADLIMITS_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyVideoGetUploadLimits : public AccessAtProtocol
{
public:
    explicit AppBskyVideoGetUploadLimits(QObject *parent = nullptr);

    void getUploadLimits();

    const bool &canUpload() const;
    const qint64 &remainingDailyVideos() const;
    const qint64 &remainingDailyBytes() const;
    const QString &message() const;
    const QString &error() const;

protected:
    virtual bool parseJson(bool success, const QString reply_json);

private:
    bool m_canUpload;
    qint64 m_remainingDailyVideos;
    qint64 m_remainingDailyBytes;
    QString m_message;
    QString m_error;
};

}

#endif // APPBSKYVIDEOGETUPLOADLIMITS_H
