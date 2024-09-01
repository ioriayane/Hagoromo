#ifndef APPBSKYNOTIFICATIONPUTPREFERENCES_H
#define APPBSKYNOTIFICATIONPUTPREFERENCES_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyNotificationPutPreferences : public AccessAtProtocol
{
public:
    explicit AppBskyNotificationPutPreferences(QObject *parent = nullptr);

    void putPreferences(const bool priority);

private:
    virtual bool parseJson(bool success, const QString reply_json);
};

}

#endif // APPBSKYNOTIFICATIONPUTPREFERENCES_H
