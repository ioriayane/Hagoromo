#ifndef APPBSKYGRAPHUNMUTEACTORLIST_H
#define APPBSKYGRAPHUNMUTEACTORLIST_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyGraphUnmuteActorList : public AccessAtProtocol
{
public:
    explicit AppBskyGraphUnmuteActorList(QObject *parent = nullptr);

    void unmuteActorList(const QString &list);

private:
    virtual bool parseJson(bool success, const QString reply_json);
};

}

#endif // APPBSKYGRAPHUNMUTEACTORLIST_H
