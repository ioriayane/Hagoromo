#ifndef CHATBSKYCONVOSENDMESSAGEBATCH_H
#define CHATBSKYCONVOSENDMESSAGEBATCH_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyConvoSendMessageBatch : public AccessAtProtocol
{
public:
    explicit ChatBskyConvoSendMessageBatch(QObject *parent = nullptr);

    void sendMessageBatch(const QJsonObject &items);

    const QList<AtProtocolType::ChatBskyConvoDefs::MessageView> &itemsMessageViewList() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::ChatBskyConvoDefs::MessageView> m_itemsMessageViewList;
};

}

#endif // CHATBSKYCONVOSENDMESSAGEBATCH_H
