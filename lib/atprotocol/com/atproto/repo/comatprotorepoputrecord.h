#ifndef COMATPROTOREPOPUTRECORD_H
#define COMATPROTOREPOPUTRECORD_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ComAtprotoRepoPutRecord : public AccessAtProtocol
{
public:
    explicit ComAtprotoRepoPutRecord(QObject *parent = nullptr);

    void putRecord(const QString &repo, const QString &collection, const QString &rkey,
                   const bool validate, const QJsonObject &record, const QString &swapRecord,
                   const QString &swapCommit);

    const QString &uri() const;
    const QString &cid() const;

protected:
    virtual bool parseJson(bool success, const QString reply_json);

private:
    QString m_uri;
    QString m_cid;
};

}

#endif // COMATPROTOREPOPUTRECORD_H
