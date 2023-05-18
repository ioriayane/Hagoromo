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

    AtProtocolType::AppBskyActorDefs::ProfileViewDetailed profileViewDetailed() const;

private:
    virtual void parseJson(const QString reply_json);

    AtProtocolType::AppBskyActorDefs::ProfileViewDetailed m_profileViewDetailed;
};

}

#endif // APPBSKYACTORGETPROFILE_H
