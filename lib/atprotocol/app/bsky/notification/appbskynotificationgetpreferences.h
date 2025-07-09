#ifndef APPBSKYNOTIFICATIONGETPREFERENCES_H
#define APPBSKYNOTIFICATIONGETPREFERENCES_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyNotificationGetPreferences : public AccessAtProtocol
{
public:
    explicit AppBskyNotificationGetPreferences(QObject *parent = nullptr);

    void getPreferences();

    const AtProtocolType::AppBskyNotificationDefs::Preferences &preferences() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::AppBskyNotificationDefs::Preferences m_preferences;
};

}

#endif // APPBSKYNOTIFICATIONGETPREFERENCES_H
