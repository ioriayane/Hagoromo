#ifndef CHATBSKYGROUPREQUESTJOIN_H
#define CHATBSKYGROUPREQUESTJOIN_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyGroupRequestJoin : public AccessAtProtocol
{
public:
    explicit ChatBskyGroupRequestJoin(QObject *parent = nullptr);

    void requestJoin(const QString &code);

    const QString &status() const;
    const AtProtocolType::ChatBskyConvoDefs::ConvoView &convo() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QString m_status;
    AtProtocolType::ChatBskyConvoDefs::ConvoView m_convo;
};

}

#endif // CHATBSKYGROUPREQUESTJOIN_H
