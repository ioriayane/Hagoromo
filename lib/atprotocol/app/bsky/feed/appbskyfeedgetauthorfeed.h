#ifndef APPBSKYFEEDGETAUTHORFEED_H
#define APPBSKYFEEDGETAUTHORFEED_H

#include "atprotocol/app/bsky/feed/appbskyfeedgettimeline.h"

namespace AtProtocolInterface {

class AppBskyFeedGetAuthorFeed : public AppBskyFeedGetTimeline
{
public:
    explicit AppBskyFeedGetAuthorFeed(QObject *parent = nullptr);

    void getAuthorFeed(const QString &actor, const qint64 limit, const QString &cursor,
                       const QString &filter, const bool includePins);
};

}

#endif // APPBSKYFEEDGETAUTHORFEED_H
