#ifndef APPBSKYACTORPUTPREFERENCES_H
#define APPBSKYACTORPUTPREFERENCES_H

#include "atprotocol/accessatprotocol.h"
#include "atprotocol/lexicons.h"

namespace AtProtocolInterface {

class AppBskyActorPutPreferences : public AccessAtProtocol
{
public:
    explicit AppBskyActorPutPreferences(QObject *parent = nullptr);

    void putPreferences(const QString &json);

private:
    virtual void parseJson(const QString reply_json);
};

}

#endif // APPBSKYACTORPUTPREFERENCES_H
