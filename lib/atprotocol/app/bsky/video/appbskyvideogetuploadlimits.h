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
    const int &remainingDailyVideos() const;
    const int &remainingDailyBytes() const;
    const QString &message() const;
    const QString &error() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    bool m_canUpload;
    int m_remainingDailyVideos;
    int m_remainingDailyBytes;
    QString m_message;
    QString m_error;
};

}

#endif // APPBSKYVIDEOGETUPLOADLIMITS_H
