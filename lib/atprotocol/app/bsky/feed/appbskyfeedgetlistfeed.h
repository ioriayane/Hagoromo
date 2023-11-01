#ifndef APPBSKYFEEDGETLISTFEED_H
#define APPBSKYFEEDGETLISTFEED_H

#include "appbskyfeedgettimeline.h"

namespace AtProtocolInterface {

class AppBskyFeedGetListFeed : public AppBskyFeedGetTimeline
{
public:
    explicit AppBskyFeedGetListFeed(QObject *parent = nullptr);

    void getTimeline() = delete;
    void getListFeed(const QString &list, const int limit, const QString &cursor);
};

}

#endif // APPBSKYFEEDGETLISTFEED_H
