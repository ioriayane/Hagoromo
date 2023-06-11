#ifndef APPBSKYFEEDGETPOSTTHREAD_H
#define APPBSKYFEEDGETPOSTTHREAD_H

#include "atprotocol/accessatprotocol.h"
#include "atprotocol/lexicons.h"

namespace AtProtocolInterface {

class AppBskyFeedGetPostThread : public AccessAtProtocol
{
public:
    explicit AppBskyFeedGetPostThread(QObject *parent = nullptr);

    void getPostThread(const QString &uri);

    const AtProtocolType::AppBskyFeedDefs::ThreadViewPost *threadViewPost() const;

private:
    virtual void parseJson(const QString reply_json);

    AtProtocolType::AppBskyFeedDefs::ThreadViewPost m_threadViewPost;
};

}

#endif // APPBSKYFEEDGETPOSTTHREAD_H
