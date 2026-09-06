#ifndef APPBSKYVIDEOUPLOADPART_H
#define APPBSKYVIDEOUPLOADPART_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyVideoUploadPart : public AccessAtProtocol
{
public:
    explicit AppBskyVideoUploadPart(QObject *parent = nullptr);

    void uploadPart();

    const int &partNumber() const;
    const int &sizeBytes() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    int m_partNumber;
    int m_sizeBytes;
};

}

#endif // APPBSKYVIDEOUPLOADPART_H
