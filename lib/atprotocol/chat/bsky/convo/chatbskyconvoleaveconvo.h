#ifndef CHATBSKYCONVOLEAVECONVO_H
#define CHATBSKYCONVOLEAVECONVO_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyConvoLeaveConvo : public AccessAtProtocol
{
public:
    explicit ChatBskyConvoLeaveConvo(QObject *parent = nullptr);

    void leaveConvo(const QString &convoId);

    const QString &convoId() const;
    const QString &rev() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QString m_convoId;
    QString m_rev;
};

}

#endif // CHATBSKYCONVOLEAVECONVO_H
