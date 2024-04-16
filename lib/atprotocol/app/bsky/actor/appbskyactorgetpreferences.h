#ifndef APPBSKYACTORGETPREFERENCES_H
#define APPBSKYACTORGETPREFERENCES_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyActorGetPreferences : public AccessAtProtocol
{
public:
    explicit AppBskyActorGetPreferences(QObject *parent = nullptr);

    void getPreferences();

    const AtProtocolType::AppBskyActorDefs::Preferences &preferences() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::AppBskyActorDefs::Preferences m_preferences;
};

}

#endif // APPBSKYACTORGETPREFERENCES_H
