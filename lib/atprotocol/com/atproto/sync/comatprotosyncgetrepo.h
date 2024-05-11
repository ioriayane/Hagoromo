#ifndef COMATPROTOSYNCGETREPO_H
#define COMATPROTOSYNCGETREPO_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ComAtprotoSyncGetRepo : public AccessAtProtocol
{
public:
    explicit ComAtprotoSyncGetRepo(QObject *parent = nullptr);

    void getRepo(const QString &did, const QString &since);

    const QByteArray &repo() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);
    virtual bool recvImage(const QByteArray &data, const QString &content_type);

    QByteArray m_repo;
};

}

#endif // COMATPROTOSYNCGETREPO_H
