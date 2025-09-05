#ifndef APPBSKYNOTIFICATIONUNREGISTERPUSH_H
#define APPBSKYNOTIFICATIONUNREGISTERPUSH_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyNotificationUnregisterPush : public AccessAtProtocol
{
public:
    explicit AppBskyNotificationUnregisterPush(QObject *parent = nullptr);

    void unregisterPush(const QString &serviceDid, const QString &token, const QString &platform,
                        const QString &appId);

private:
    virtual bool parseJson(bool success, const QString reply_json);
};

}

#endif // APPBSKYNOTIFICATIONUNREGISTERPUSH_H
