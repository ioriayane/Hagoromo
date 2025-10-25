#ifndef APPBSKYFEEDGETLISTFEED_H
#define APPBSKYFEEDGETLISTFEED_H

#include "atprotocol/app/bsky/feed/appbskyfeedgettimeline.h"

namespace AtProtocolInterface {

class AppBskyFeedGetListFeed : public AppBskyFeedGetTimeline
{
public:
    explicit AppBskyFeedGetListFeed(QObject *parent = nullptr);

    void getListFeed(const QString &list, const qint64 limit, const QString &cursor);
};

}

#endif // APPBSKYFEEDGETLISTFEED_H
