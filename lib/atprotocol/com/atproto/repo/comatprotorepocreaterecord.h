#ifndef COMATPROTOREPOCREATERECORD_H
#define COMATPROTOREPOCREATERECORD_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ComAtprotoRepoCreateRecord : public AccessAtProtocol
{
public:
    explicit ComAtprotoRepoCreateRecord(QObject *parent = nullptr);

    void createRecord(const QString &repo, const QString &collection, const QString &rkey,
                      const bool validate, const QJsonObject &record, const QString &swapCommit);

    const QString &uri() const;
    const QString &cid() const;

protected:
    virtual bool parseJson(bool success, const QString reply_json);

private:
    QString m_uri;
    QString m_cid;
};

}

#endif // COMATPROTOREPOCREATERECORD_H
