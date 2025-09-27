#ifndef APPBSKYUNSPECCEDGETONBOARDINGSUGGESTEDSTARTERPACKSSKELETON_H
#define APPBSKYUNSPECCEDGETONBOARDINGSUGGESTEDSTARTERPACKSSKELETON_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyUnspeccedGetOnboardingSuggestedStarterPacksSkeleton : public AccessAtProtocol
{
public:
    explicit AppBskyUnspeccedGetOnboardingSuggestedStarterPacksSkeleton(QObject *parent = nullptr);

    void getOnboardingSuggestedStarterPacksSkeleton(const QString &viewer, const int limit);

    const QStringList &starterPacksList() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QStringList m_starterPacksList;
};

}

#endif // APPBSKYUNSPECCEDGETONBOARDINGSUGGESTEDSTARTERPACKSSKELETON_H
