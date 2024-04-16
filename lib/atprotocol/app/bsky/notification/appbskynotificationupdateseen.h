#ifndef APPBSKYNOTIFICATIONUPDATESEEN_H
#define APPBSKYNOTIFICATIONUPDATESEEN_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyNotificationUpdateSeen : public AccessAtProtocol
{
public:
    explicit AppBskyNotificationUpdateSeen(QObject *parent = nullptr);

    void updateSeen(const QString &seenAt);

private:
    virtual bool parseJson(bool success, const QString reply_json);
};

}

#endif // APPBSKYNOTIFICATIONUPDATESEEN_H
