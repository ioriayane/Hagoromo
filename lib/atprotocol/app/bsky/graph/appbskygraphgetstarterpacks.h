#ifndef APPBSKYGRAPHGETSTARTERPACKS_H
#define APPBSKYGRAPHGETSTARTERPACKS_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyGraphGetStarterPacks : public AccessAtProtocol
{
public:
    explicit AppBskyGraphGetStarterPacks(QObject *parent = nullptr);

    void getStarterPacks(const QList<QString> &uris);

    const QList<AtProtocolType::AppBskyGraphDefs::StarterPackViewBasic> &starterPacksList() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::AppBskyGraphDefs::StarterPackViewBasic> m_starterPacksList;
};

}

#endif // APPBSKYGRAPHGETSTARTERPACKS_H
