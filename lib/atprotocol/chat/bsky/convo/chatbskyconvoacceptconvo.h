#ifndef CHATBSKYCONVOACCEPTCONVO_H
#define CHATBSKYCONVOACCEPTCONVO_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyConvoAcceptConvo : public AccessAtProtocol
{
public:
    explicit ChatBskyConvoAcceptConvo(QObject *parent = nullptr);

    void acceptConvo(const QString &convoId);

    const QString &rev() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QString m_rev;
};

}

#endif // CHATBSKYCONVOACCEPTCONVO_H
