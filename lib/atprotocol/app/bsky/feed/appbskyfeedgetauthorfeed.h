#ifndef APPBSKYFEEDGETAUTHORFEED_H
#define APPBSKYFEEDGETAUTHORFEED_H

#include "appbskyfeedgettimeline.h"

namespace AtProtocolInterface {

// app.bsky.feed.getAuthorFeedのoutputがapp.bsky.feed.getTimelineと同じで
// app.bsky.feed.defs#feedViewPostの間はAppBskyFeedGetTimelineを継承する

class AppBskyFeedGetAuthorFeed : public AppBskyFeedGetTimeline
{
public:
    explicit AppBskyFeedGetAuthorFeed(QObject *parent = nullptr);

    void getTimeline() = delete;
    void getAuthorFeed(const QString &actor, const int limit, const QString &cursor);

private:
};

}

#endif // APPBSKYFEEDGETAUTHORFEED_H
