#ifndef APPBSKYACTORGETPROFILES_H
#define APPBSKYACTORGETPROFILES_H

#include "atprotocol/accessatprotocol.h"
#include "atprotocol/lexicons.h"

namespace AtProtocolInterface {

class AppBskyActorGetProfiles : public AccessAtProtocol
{
public:
    explicit AppBskyActorGetProfiles(QObject *parent = nullptr);

    void getProfiles(const QList<QString> &actors);

    const QList<AtProtocolType::AppBskyActorDefs::ProfileViewDetailed> *
    profileViewDetaileds() const;

private:
    virtual void parseJson(const QString reply_json);
    QList<AtProtocolType::AppBskyActorDefs::ProfileViewDetailed> m_profileViewDetaileds;
};

}

#endif // APPBSKYACTORGETPROFILES_H
