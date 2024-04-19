#ifndef COMATPROTOSYNCGETBLOB_H
#define COMATPROTOSYNCGETBLOB_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ComAtprotoSyncGetBlob : public AccessAtProtocol
{
public:
    explicit ComAtprotoSyncGetBlob(QObject *parent = nullptr);

    void getBlob(const QString &did, const QString &cid);

    const QByteArray &blobData() const;
    const QString &extension() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);
    virtual bool recvImage(const QByteArray &data, const QString &content_type);

    QByteArray m_blobData;
    QString m_extension;
};

}

#endif // COMATPROTOSYNCGETBLOB_H
