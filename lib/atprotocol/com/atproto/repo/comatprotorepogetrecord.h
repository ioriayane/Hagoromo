#ifndef COMATPROTOREPOGETRECORD_H
#define COMATPROTOREPOGETRECORD_H

#include "atprotocol/accessatprotocol.h"
#include "atprotocol/lexicons.h"

namespace AtProtocolInterface {

class ComAtprotoRepoGetRecord : public AccessAtProtocol
{
public:
    explicit ComAtprotoRepoGetRecord(QObject *parent = nullptr);

    void getRecord(const QString &repo, const QString &collection, const QString &rkey,
                   const QString &cid);

    void profile(const QString &did);
    void list(const QString &did, const QString &rkey);

    QString cid() const;
    QString uri() const;
    QVariant record() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QString m_cid;
    QString m_uri;
    QVariant m_record;
};

}

#endif // COMATPROTOREPOGETRECORD_H
