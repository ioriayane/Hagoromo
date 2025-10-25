#ifndef CHATBSKYCONVOLISTCONVOS_H
#define CHATBSKYCONVOLISTCONVOS_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyConvoListConvos : public AccessAtProtocol
{
public:
    explicit ChatBskyConvoListConvos(QObject *parent = nullptr);

    void listConvos(const qint64 limit, const QString &cursor, const QString &readState,
                    const QString &status);

    const QList<AtProtocolType::ChatBskyConvoDefs::ConvoView> &convosList() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::ChatBskyConvoDefs::ConvoView> m_convosList;
};

}

#endif // CHATBSKYCONVOLISTCONVOS_H
