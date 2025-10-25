#ifndef APPBSKYFEEDGETPOSTTHREAD_H
#define APPBSKYFEEDGETPOSTTHREAD_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyFeedGetPostThread : public AccessAtProtocol
{
public:
    explicit AppBskyFeedGetPostThread(QObject *parent = nullptr);

    void getPostThread(const QString &uri, const qint64 depth, const qint64 parentHeight);

    const AtProtocolType::AppBskyFeedDefs::ThreadViewPost &threadViewPost() const;
    const AtProtocolType::AppBskyFeedDefs::NotFoundPost &notFoundPost() const;
    const AtProtocolType::AppBskyFeedDefs::BlockedPost &blockedPost() const;
    const AtProtocolType::AppBskyFeedDefs::ThreadgateView &threadgate() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::AppBskyFeedDefs::ThreadViewPost m_threadViewPost;
    AtProtocolType::AppBskyFeedDefs::NotFoundPost m_notFoundPost;
    AtProtocolType::AppBskyFeedDefs::BlockedPost m_blockedPost;
    AtProtocolType::AppBskyFeedDefs::ThreadgateView m_threadgate;
};

}

#endif // APPBSKYFEEDGETPOSTTHREAD_H
