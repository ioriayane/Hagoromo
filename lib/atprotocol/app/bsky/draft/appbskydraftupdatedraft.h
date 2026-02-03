#ifndef APPBSKYDRAFTUPDATEDRAFT_H
#define APPBSKYDRAFTUPDATEDRAFT_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyDraftUpdateDraft : public AccessAtProtocol
{
public:
    explicit AppBskyDraftUpdateDraft(QObject *parent = nullptr);

    void updateDraft(const QJsonObject &draft);

private:
    virtual bool parseJson(bool success, const QString reply_json);
};

}

#endif // APPBSKYDRAFTUPDATEDRAFT_H
