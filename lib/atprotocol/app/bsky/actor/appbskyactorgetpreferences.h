#ifndef APPBSKYACTORGETPREFERENCES_H
#define APPBSKYACTORGETPREFERENCES_H

#include "atprotocol/accessatprotocol.h"
#include "atprotocol/lexicons.h"

namespace AtProtocolInterface {

class AppBskyActorGetPreferences : public AccessAtProtocol
{
public:
    explicit AppBskyActorGetPreferences(QObject *parent = nullptr);

    bool getPreferences();

    const QList<AtProtocolType::AppBskyActorDefs::SavedFeedsPref> *savedFeedsPrefList() const;
    const QList<AtProtocolType::AppBskyActorDefs::ContentLabelPref> *contentLabelPrefList() const;
    const AtProtocolType::AppBskyActorDefs::AdultContentPref adultContentPref() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::AppBskyActorDefs::SavedFeedsPref> m_savedFeedsPrefList;
    QList<AtProtocolType::AppBskyActorDefs::ContentLabelPref> m_contentLabelPrefList;
    AtProtocolType::AppBskyActorDefs::AdultContentPref m_adultContentPref;
};

}

#endif // APPBSKYACTORGETPREFERENCES_H
