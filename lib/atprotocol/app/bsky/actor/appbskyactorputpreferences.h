#ifndef APPBSKYACTORPUTPREFERENCES_H
#define APPBSKYACTORPUTPREFERENCES_H

#include "atprotocol/accessatprotocol.h"
#include "atprotocol/lexicons.h"

namespace AtProtocolInterface {

class AppBskyActorPutPreferences : public AccessAtProtocol
{
public:
    explicit AppBskyActorPutPreferences(QObject *parent = nullptr);

    bool putPreferences(const QString &json);

private:
    virtual bool parseJson(bool success, const QString reply_json);
};

}

#endif // APPBSKYACTORPUTPREFERENCES_H
