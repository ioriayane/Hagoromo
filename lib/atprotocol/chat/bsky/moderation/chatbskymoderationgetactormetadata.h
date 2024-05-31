#ifndef CHATBSKYMODERATIONGETACTORMETADATA_H
#define CHATBSKYMODERATIONGETACTORMETADATA_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ChatBskyModerationGetActorMetadata : public AccessAtProtocol
{
public:
    explicit ChatBskyModerationGetActorMetadata(QObject *parent = nullptr);

    void getActorMetadata(const QString &actor);

    const AtProtocolType::ChatBskyModerationGetActorMetadata::Metadata &day() const;
    const AtProtocolType::ChatBskyModerationGetActorMetadata::Metadata &month() const;
    const AtProtocolType::ChatBskyModerationGetActorMetadata::Metadata &all() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::ChatBskyModerationGetActorMetadata::Metadata m_day;
    AtProtocolType::ChatBskyModerationGetActorMetadata::Metadata m_month;
    AtProtocolType::ChatBskyModerationGetActorMetadata::Metadata m_all;
};

}

#endif // CHATBSKYMODERATIONGETACTORMETADATA_H
