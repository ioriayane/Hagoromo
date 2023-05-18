#ifndef APPBSKYFEEDGETPOSTTHREAD_H
#define APPBSKYFEEDGETPOSTTHREAD_H

#include "accessatprotocol.h"
#include "../atprotocol/lexicons.h"

namespace AtProtocolInterface {

class AppBskyFeedGetPostThread : public AccessAtProtocol
{
public:
    explicit AppBskyFeedGetPostThread(QObject *parent = nullptr);

    void getPostThread(const QString &uri);

    AtProtocolType::AppBskyFeedDefs::ThreadViewPost *threadViewPost();

private:
    virtual void parseJson(const QString reply_json);

    AtProtocolType::AppBskyFeedDefs::ThreadViewPost m_threadViewPost;
};

}

#endif // APPBSKYFEEDGETPOSTTHREAD_H
