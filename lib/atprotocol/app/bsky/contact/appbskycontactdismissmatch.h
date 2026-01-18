#ifndef APPBSKYCONTACTDISMISSMATCH_H
#define APPBSKYCONTACTDISMISSMATCH_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyContactDismissMatch : public AccessAtProtocol
{
public:
    explicit AppBskyContactDismissMatch(QObject *parent = nullptr);

    void dismissMatch(const QString &subject);

private:
    virtual bool parseJson(bool success, const QString reply_json);
};

}

#endif // APPBSKYCONTACTDISMISSMATCH_H
