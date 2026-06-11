#ifndef CHATBSKYCONVOGETUNREADCOUNTS_H
#define CHATBSKYCONVOGETUNREADCOUNTS_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyConvoGetUnreadCounts : public AccessAtProtocol
{
public:
    explicit ChatBskyConvoGetUnreadCounts(QObject *parent = nullptr);

    void getUnreadCounts();

    const int &unreadAcceptedConvos() const;
    const int &unreadRequestConvos() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    int m_unreadAcceptedConvos;
    int m_unreadRequestConvos;
};

}

#endif // CHATBSKYCONVOGETUNREADCOUNTS_H
