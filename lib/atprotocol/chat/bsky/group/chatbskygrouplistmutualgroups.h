#ifndef CHATBSKYGROUPLISTMUTUALGROUPS_H
#define CHATBSKYGROUPLISTMUTUALGROUPS_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyGroupListMutualGroups : public AccessAtProtocol
{
public:
    explicit ChatBskyGroupListMutualGroups(QObject *parent = nullptr);

    void listMutualGroups(const QString &subject, const int limit, const QString &cursor);

    const QList<AtProtocolType::ChatBskyConvoDefs::ConvoView> &convosList() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::ChatBskyConvoDefs::ConvoView> m_convosList;
};

}

#endif // CHATBSKYGROUPLISTMUTUALGROUPS_H
