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

    void setReply(const QString &parent_cid, const QString &parent_uri, const QString &root_cid,
                  const QString &root_uri);
    void setQuote(const QString &cid, const QString &uri);

private:
    virtual void parseJson(const QString reply_json);

    AtProtocolType::ComAtprotoRepoStrongRef::Main m_replyParent;
    AtProtocolType::ComAtprotoRepoStrongRef::Main m_replyRoot;
    AtProtocolType::ComAtprotoRepoStrongRef::Main m_embedQuote;
};

}

#endif // COMATPROTOREPOCREATERECORD_H
