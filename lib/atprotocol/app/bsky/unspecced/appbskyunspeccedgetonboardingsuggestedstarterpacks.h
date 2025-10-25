#ifndef APPBSKYUNSPECCEDGETONBOARDINGSUGGESTEDSTARTERPACKS_H
#define APPBSKYUNSPECCEDGETONBOARDINGSUGGESTEDSTARTERPACKS_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyUnspeccedGetOnboardingSuggestedStarterPacks : public AccessAtProtocol
{
public:
    explicit AppBskyUnspeccedGetOnboardingSuggestedStarterPacks(QObject *parent = nullptr);

    void getOnboardingSuggestedStarterPacks(const qint64 limit);

    const QList<AtProtocolType::AppBskyGraphDefs::StarterPackView> &starterPacksList() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::AppBskyGraphDefs::StarterPackView> m_starterPacksList;
};

}

#endif // APPBSKYUNSPECCEDGETONBOARDINGSUGGESTEDSTARTERPACKS_H
