#ifndef APPBSKYGRAPHGETSTARTERPACK_H
#define APPBSKYGRAPHGETSTARTERPACK_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyGraphGetStarterPack : public AccessAtProtocol
{
public:
    explicit AppBskyGraphGetStarterPack(QObject *parent = nullptr);

    void getStarterPack(const QString &starterPack);

    const AtProtocolType::AppBskyGraphDefs::StarterPackView &starterPack() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::AppBskyGraphDefs::StarterPackView m_starterPack;
};

}

#endif // APPBSKYGRAPHGETSTARTERPACK_H
