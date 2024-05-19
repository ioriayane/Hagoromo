#ifndef CHATBSKYCONVODELETEMESSAGEFORSELF_H
#define CHATBSKYCONVODELETEMESSAGEFORSELF_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyConvoDeleteMessageForSelf : public AccessAtProtocol
{
public:
    explicit ChatBskyConvoDeleteMessageForSelf(QObject *parent = nullptr);

    void deleteMessageForSelf(const QString &convoId, const QString &messageId);

    const AtProtocolType::ChatBskyConvoDefs::DeletedMessageView &deletedMessageView() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::ChatBskyConvoDefs::DeletedMessageView m_deletedMessageView;
};

}

#endif // CHATBSKYCONVODELETEMESSAGEFORSELF_H
