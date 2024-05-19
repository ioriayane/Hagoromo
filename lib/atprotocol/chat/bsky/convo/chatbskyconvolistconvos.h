#ifndef CHATBSKYCONVOLISTCONVOS_H
#define CHATBSKYCONVOLISTCONVOS_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyConvoListConvos : public AccessAtProtocol
{
public:
    explicit ChatBskyConvoListConvos(QObject *parent = nullptr);

    void listConvos(const int limit, const QString &cursor);

    const QList<AtProtocolType::ChatBskyConvoDefs::ConvoView> &convosConvoViewList() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::ChatBskyConvoDefs::ConvoView> m_convosConvoViewList;
};

}

#endif // CHATBSKYCONVOLISTCONVOS_H
