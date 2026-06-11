#ifndef INTERNALBSKYACTORGETPROFILES_H
#define INTERNALBSKYACTORGETPROFILES_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class InternalBskyActorGetProfiles : public AccessAtProtocol
{
public:
    explicit InternalBskyActorGetProfiles(QObject *parent = nullptr);

    void getProfiles(const QList<QString> &dids, const QString &viewer,
                     const QList<QString> &socialProof);

    const QList<AtProtocolType::AppBskyActorDefs::ProfileViewDetailed> &profilesList() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::AppBskyActorDefs::ProfileViewDetailed> m_profilesList;
};

}

#endif // INTERNALBSKYACTORGETPROFILES_H
