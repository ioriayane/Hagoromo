#ifndef CHATBSKYACTOREXPORTACCOUNTDATA_H
#define CHATBSKYACTOREXPORTACCOUNTDATA_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyActorExportAccountData : public AccessAtProtocol
{
public:
    explicit ChatBskyActorExportAccountData(QObject *parent = nullptr);

    void exportAccountData();

private:
    virtual bool parseJson(bool success, const QString reply_json);
};

}

#endif // CHATBSKYACTOREXPORTACCOUNTDATA_H
