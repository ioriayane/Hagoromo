#ifndef COMATPROTOMODERATIONCREATEREPORT_H
#define COMATPROTOMODERATIONCREATEREPORT_H

#include "atprotocol/accessatprotocol.h"
#include "atprotocol/lexicons.h"

namespace AtProtocolInterface {

class ComAtprotoModerationCreateReport : public AccessAtProtocol
{
public:
    explicit ComAtprotoModerationCreateReport(QObject *parent = nullptr);

    void reportPost(const QString &uri, const QString &cid, const QString &reason);
    void reportAccount(const QString &did, const QString &reason);

private:
    virtual void parseJson(bool success, const QString reply_json);
};

}

#endif // COMATPROTOMODERATIONCREATEREPORT_H
