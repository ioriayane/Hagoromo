#ifndef COMATPROTOSYNCGETBLOB_H
#define COMATPROTOSYNCGETBLOB_H

#include "atprotocol/accessatprotocol.h"
#include "atprotocol/lexicons.h"

namespace AtProtocolInterface {

class ComAtprotoSyncGetBlob : public AccessAtProtocol
{
public:
    explicit ComAtprotoSyncGetBlob(QObject *parent = nullptr);

    void getBlob(const QString &did, const QString &cid);

    const QByteArray &blobData() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);
    virtual bool recvImage(const QByteArray &data);

    QByteArray m_blobData;
};

}

#endif // COMATPROTOSYNCGETBLOB_H
