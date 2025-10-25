#ifndef APPBSKYGRAPHGETSTARTERPACKSWITHMEMBERSHIP_H
#define APPBSKYGRAPHGETSTARTERPACKSWITHMEMBERSHIP_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyGraphGetStarterPacksWithMembership : public AccessAtProtocol
{
public:
    explicit AppBskyGraphGetStarterPacksWithMembership(QObject *parent = nullptr);

    void getStarterPacksWithMembership(const QString &actor, const qint64 limit,
                                       const QString &cursor);

    const QList<
            AtProtocolType::AppBskyGraphGetStarterPacksWithMembership::StarterPackWithMembership> &
    starterPacksWithMembershipList() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::AppBskyGraphGetStarterPacksWithMembership::StarterPackWithMembership>
            m_starterPacksWithMembershipList;
};

}

#endif // APPBSKYGRAPHGETSTARTERPACKSWITHMEMBERSHIP_H
