#ifndef COMATPROTOREPOCREATERECORD_H
#define COMATPROTOREPOCREATERECORD_H

#include "accessatprotocol.h"
#include "../atprotocol/lexicons.h"

namespace AtProtocolInterface {

class ComAtprotoRepoCreateRecord : public AccessAtProtocol
{
public:
    explicit ComAtprotoRepoCreateRecord(QObject *parent = nullptr);

    void post(const QString &text);
    void repost(const QString &cid, const QString &uri);
    void like(const QString &cid, const QString &uri);

    void setQuote(const QString &cid, const QString &uri);

private:
    virtual void parseJson(const QString reply_json);

    AtProtocolType::ComAtprotoRepoStrongRef::Main m_embedQuote;
};

}

#endif // COMATPROTOREPOCREATERECORD_H
