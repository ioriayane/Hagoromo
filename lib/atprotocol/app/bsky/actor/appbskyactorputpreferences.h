#ifndef APPBSKYACTORPUTPREFERENCES_H
#define APPBSKYACTORPUTPREFERENCES_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyActorPutPreferences : public AccessAtProtocol
{
public:
    explicit AppBskyActorPutPreferences(QObject *parent = nullptr);

    void putPreferences(const QJsonArray &preferences);

protected:
    virtual bool parseJson(bool success, const QString reply_json);
};

}

#endif // APPBSKYACTORPUTPREFERENCES_H
