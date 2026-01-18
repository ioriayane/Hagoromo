#ifndef APPBSKYCONTACTREMOVEDATA_H
#define APPBSKYCONTACTREMOVEDATA_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyContactRemoveData : public AccessAtProtocol
{
public:
    explicit AppBskyContactRemoveData(QObject *parent = nullptr);

    void removeData();

private:
    virtual bool parseJson(bool success, const QString reply_json);
};

}

#endif // APPBSKYCONTACTREMOVEDATA_H
