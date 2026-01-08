#ifndef CHATBSKYCONVOUPDATEALLREAD_H
#define CHATBSKYCONVOUPDATEALLREAD_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyConvoUpdateAllRead : public AccessAtProtocol
{
public:
    explicit ChatBskyConvoUpdateAllRead(QObject *parent = nullptr);

    void updateAllRead(const QString &status);

    const qint64 &updatedCount() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    qint64 m_updatedCount;
};

}

#endif // CHATBSKYCONVOUPDATEALLREAD_H
