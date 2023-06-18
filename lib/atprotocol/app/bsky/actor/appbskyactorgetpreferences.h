#ifndef APPBSKYACTORGETPREFERENCES_H
#define APPBSKYACTORGETPREFERENCES_H

#include "atprotocol/accessatprotocol.h"
#include "atprotocol/lexicons.h"

namespace AtProtocolInterface {

class AppBskyActorGetPreferences : public AccessAtProtocol
{
public:
    explicit AppBskyActorGetPreferences(QObject *parent = nullptr);

    void getPreferences();

    const QList<AtProtocolType::AppBskyActorDefs::SavedFeedsPref> *savedFeedsPrefList() const;

private:
    virtual void parseJson(const QString reply_json);

    QList<AtProtocolType::AppBskyActorDefs::SavedFeedsPref> m_savedFeedsPrefList;
};

}

#endif // APPBSKYACTORGETPREFERENCES_H
