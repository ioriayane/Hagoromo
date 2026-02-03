#ifndef APPBSKYCONTACTIMPORTCONTACTS_H
#define APPBSKYCONTACTIMPORTCONTACTS_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyContactImportContacts : public AccessAtProtocol
{
public:
    explicit AppBskyContactImportContacts(QObject *parent = nullptr);

    void importContacts(const QString &token, const QList<QString> &contacts);

    const QList<AtProtocolType::AppBskyContactDefs::MatchAndContactIndex> &
    matchesAndContactIndexesList() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::AppBskyContactDefs::MatchAndContactIndex> m_matchesAndContactIndexesList;
};

}

#endif // APPBSKYCONTACTIMPORTCONTACTS_H
