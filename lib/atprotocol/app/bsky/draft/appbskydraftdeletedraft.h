#ifndef APPBSKYDRAFTDELETEDRAFT_H
#define APPBSKYDRAFTDELETEDRAFT_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyDraftDeleteDraft : public AccessAtProtocol
{
public:
    explicit AppBskyDraftDeleteDraft(QObject *parent = nullptr);

    void deleteDraft(const QString &id);

private:
    virtual bool parseJson(bool success, const QString reply_json);
};

}

#endif // APPBSKYDRAFTDELETEDRAFT_H
