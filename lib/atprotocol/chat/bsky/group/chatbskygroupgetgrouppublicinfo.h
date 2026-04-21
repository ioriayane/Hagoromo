#ifndef CHATBSKYGROUPGETGROUPPUBLICINFO_H
#define CHATBSKYGROUPGETGROUPPUBLICINFO_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyGroupGetGroupPublicInfo : public AccessAtProtocol
{
public:
    explicit ChatBskyGroupGetGroupPublicInfo(QObject *parent = nullptr);

    void getGroupPublicInfo(const QString &code);

    const AtProtocolType::ChatBskyGroupDefs::GroupPublicView &group() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::ChatBskyGroupDefs::GroupPublicView m_group;
};

}

#endif // CHATBSKYGROUPGETGROUPPUBLICINFO_H
