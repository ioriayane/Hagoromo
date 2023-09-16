#ifndef APPBSKYGRAPHUNMUTEACTOR_H
#define APPBSKYGRAPHUNMUTEACTOR_H

#include "atprotocol/accessatprotocol.h"
#include "atprotocol/lexicons.h"

namespace AtProtocolInterface {

class AppBskyGraphUnmuteActor : public AccessAtProtocol
{
public:
    explicit AppBskyGraphUnmuteActor(QObject *parent = nullptr);

    void unmuteActor(const QString &actor);

private:
    virtual bool parseJson(bool success, const QString reply_json);
};

}

#endif // APPBSKYGRAPHUNMUTEACTOR_H
