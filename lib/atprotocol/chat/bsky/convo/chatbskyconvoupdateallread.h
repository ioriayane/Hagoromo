#ifndef CHATBSKYCONVOUPDATEALLREAD_H
#define CHATBSKYCONVOUPDATEALLREAD_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyConvoUpdateAllRead : public AccessAtProtocol
{
public:
    explicit ChatBskyConvoUpdateAllRead(QObject *parent = nullptr);

    void updateAllRead(const QString &status);

    const int &updatedCount() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    int m_updatedCount;
};

}

#endif // CHATBSKYCONVOUPDATEALLREAD_H
