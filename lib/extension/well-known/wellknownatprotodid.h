#ifndef WELLKNOWNATPROTODID_H
#define WELLKNOWNATPROTODID_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

// ハンドルのHTTPS well-knownによる解決(https://<handle>/.well-known/atproto-did)
class WellKnownAtprotoDid : public AccessAtProtocol
{
public:
    explicit WellKnownAtprotoDid(QObject *parent = nullptr);

    void atprotoDid();

    QString did() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);
    virtual bool recvImage(const QByteArray &data, const QString &content_type);
    virtual bool isRawContentType(const QString &content_type) const;

    QString m_did;
};

}

#endif // WELLKNOWNATPROTODID_H
