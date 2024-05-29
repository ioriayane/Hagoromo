#ifndef CHATBSKYCONVOSENDMESSAGE_H
#define CHATBSKYCONVOSENDMESSAGE_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyConvoSendMessage : public AccessAtProtocol
{
public:
    explicit ChatBskyConvoSendMessage(QObject *parent = nullptr);

    void sendMessage(const QString &convoId, const QJsonObject &message);

    const AtProtocolType::ChatBskyConvoDefs::MessageView &messageView() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::ChatBskyConvoDefs::MessageView m_messageView;
};

}

#endif // CHATBSKYCONVOSENDMESSAGE_H
