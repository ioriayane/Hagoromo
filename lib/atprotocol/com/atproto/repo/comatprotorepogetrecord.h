#ifndef COMATPROTOREPOGETRECORD_H
#define COMATPROTOREPOGETRECORD_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ComAtprotoRepoGetRecord : public AccessAtProtocol
{
public:
    explicit ComAtprotoRepoGetRecord(QObject *parent = nullptr);

    void getRecord(const QString &repo, const QString &collection, const QString &rkey,
                   const QString &cid);

    const QString &uri() const;
    const QString &cid() const;
    const QVariant &value() const;

protected:
    virtual bool parseJson(bool success, const QString reply_json);

private:
    QString m_uri;
    QString m_cid;
    QVariant m_value;
};

}

#endif // COMATPROTOREPOGETRECORD_H
