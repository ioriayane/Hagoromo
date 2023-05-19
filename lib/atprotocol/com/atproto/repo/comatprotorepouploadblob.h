#ifndef COMATPROTOREPOUPLOADBLOB_H
#define COMATPROTOREPOUPLOADBLOB_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ComAtprotoRepoUploadBlob : public AccessAtProtocol
{
public:
    explicit ComAtprotoRepoUploadBlob(QObject *parent = nullptr);

    void uploadBlob(const QString &path);

    QString cid() const;
    QString mimeType() const;
    int size() const;

private:
    virtual void parseJson(const QString reply_json);

    QString m_cid;
    QString m_mimeType;
    int m_size;
};

}

#endif // COMATPROTOREPOUPLOADBLOB_H
