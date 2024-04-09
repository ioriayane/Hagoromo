#ifndef APPBSKYACTORGETPROFILE_H
#define APPBSKYACTORGETPROFILE_H

#include "atprotocol/accessatprotocol.h"
#include "atprotocol/lexicons.h"

namespace AtProtocolInterface {

class AppBskyActorGetProfile : public AccessAtProtocol
{
public:
    explicit AppBskyActorGetProfile(QObject *parent = nullptr);

    void getProfile(const QString &actor);

    const AtProtocolType::AppBskyActorDefs::ProfileViewDetailed &profileViewDetailed() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::AppBskyActorDefs::ProfileViewDetailed m_profileViewDetailed;
};

}

#endif // APPBSKYACTORGETPROFILE_H
