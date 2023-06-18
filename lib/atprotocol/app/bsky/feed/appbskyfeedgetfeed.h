#ifndef APPBSKYFEEDGETFEED_H
#define APPBSKYFEEDGETFEED_H

#include "atprotocol/app/bsky/feed/appbskyfeedgettimeline.h"

namespace AtProtocolInterface {

class AppBskyFeedGetFeed : public AppBskyFeedGetTimeline
{
public:
    explicit AppBskyFeedGetFeed(QObject *parent = nullptr);

    void getFeed(const QString &feed, const int limit, const QString &cursor);
};

}

#endif // APPBSKYFEEDGETFEED_H
