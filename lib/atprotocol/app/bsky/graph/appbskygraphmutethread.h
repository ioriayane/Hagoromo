#ifndef APPBSKYGRAPHMUTETHREAD_H
#define APPBSKYGRAPHMUTETHREAD_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyGraphMuteThread : public AccessAtProtocol
{
public:
    explicit AppBskyGraphMuteThread(QObject *parent = nullptr);

    void muteThread(const QString &root);

private:
    virtual bool parseJson(bool success, const QString reply_json);
};

}

#endif // APPBSKYGRAPHMUTETHREAD_H
