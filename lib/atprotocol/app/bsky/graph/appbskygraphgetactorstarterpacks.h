#ifndef APPBSKYGRAPHGETACTORSTARTERPACKS_H
#define APPBSKYGRAPHGETACTORSTARTERPACKS_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyGraphGetActorStarterPacks : public AccessAtProtocol
{
public:
    explicit AppBskyGraphGetActorStarterPacks(QObject *parent = nullptr);

    void getActorStarterPacks(const QString &actor, const int limit, const QString &cursor);

    const QList<AtProtocolType::AppBskyGraphDefs::StarterPackViewBasic> &starterPacksList() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::AppBskyGraphDefs::StarterPackViewBasic> m_starterPacksList;
};

}

#endif // APPBSKYGRAPHGETACTORSTARTERPACKS_H
