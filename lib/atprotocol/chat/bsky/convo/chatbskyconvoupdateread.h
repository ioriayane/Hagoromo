#ifndef CHATBSKYCONVOUPDATEREAD_H
#define CHATBSKYCONVOUPDATEREAD_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyConvoUpdateRead : public AccessAtProtocol
{
public:
    explicit ChatBskyConvoUpdateRead(QObject *parent = nullptr);

    void updateRead(const QString &convoId, const QString &messageId);

    const AtProtocolType::ChatBskyConvoDefs::ConvoView &convo() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::ChatBskyConvoDefs::ConvoView m_convo;
};

}

#endif // CHATBSKYCONVOUPDATEREAD_H
