#ifndef APPBSKYGRAPHMUTEACTORLIST_H
#define APPBSKYGRAPHMUTEACTORLIST_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyGraphMuteActorList : public AccessAtProtocol
{
public:
    explicit AppBskyGraphMuteActorList(QObject *parent = nullptr);

    void muteActorList(const QString &list);

private:
    virtual bool parseJson(bool success, const QString reply_json);
};

}

#endif // APPBSKYGRAPHMUTEACTORLIST_H
