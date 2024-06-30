#ifndef APPBSKYGRAPHUNMUTETHREAD_H
#define APPBSKYGRAPHUNMUTETHREAD_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyGraphUnmuteThread : public AccessAtProtocol
{
public:
    explicit AppBskyGraphUnmuteThread(QObject *parent = nullptr);

    void unmuteThread(const QString &root);

private:
    virtual bool parseJson(bool success, const QString reply_json);
};

}

#endif // APPBSKYGRAPHUNMUTETHREAD_H
