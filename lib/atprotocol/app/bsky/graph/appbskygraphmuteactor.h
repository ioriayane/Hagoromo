#ifndef APPBSKYGRAPHMUTEACTOR_H
#define APPBSKYGRAPHMUTEACTOR_H

#include "atprotocol/accessatprotocol.h"
#include "atprotocol/lexicons.h"

namespace AtProtocolInterface {

class AppBskyGraphMuteActor : public AccessAtProtocol
{
public:
    explicit AppBskyGraphMuteActor(QObject *parent = nullptr);

    void muteActor(const QString &actor);

private:
    virtual void parseJson(bool success, const QString reply_json);
};

}

#endif // APPBSKYGRAPHMUTEACTOR_H
