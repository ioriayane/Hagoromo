#ifndef APPBSKYCONTACTGETSYNCSTATUS_H
#define APPBSKYCONTACTGETSYNCSTATUS_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyContactGetSyncStatus : public AccessAtProtocol
{
public:
    explicit AppBskyContactGetSyncStatus(QObject *parent = nullptr);

    void getSyncStatus();

    const AtProtocolType::AppBskyContactDefs::SyncStatus &syncStatus() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::AppBskyContactDefs::SyncStatus m_syncStatus;
};

}

#endif // APPBSKYCONTACTGETSYNCSTATUS_H
