#ifndef APPBSKYACTORGETPROFILES_H
#define APPBSKYACTORGETPROFILES_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyActorGetProfiles : public AccessAtProtocol
{
public:
    explicit AppBskyActorGetProfiles(QObject *parent = nullptr);

    void getProfiles(const QList<QString> &actors);

    const QList<AtProtocolType::AppBskyActorDefs::ProfileViewDetailed> &profilesList() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::AppBskyActorDefs::ProfileViewDetailed> m_profilesList;
};

}

#endif // APPBSKYACTORGETPROFILES_H
