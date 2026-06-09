#ifndef CHATBSKYMODERATIONGETCONVOS_H
#define CHATBSKYMODERATIONGETCONVOS_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyModerationGetConvos : public AccessAtProtocol
{
public:
    explicit ChatBskyModerationGetConvos(QObject *parent = nullptr);

    void getConvos(const QList<QString> &convoIds);

    const QList<AtProtocolType::ChatBskyModerationDefs::ConvoView> &convosList() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::ChatBskyModerationDefs::ConvoView> m_convosList;
};

}

#endif // CHATBSKYMODERATIONGETCONVOS_H
