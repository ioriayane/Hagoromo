#ifndef APPBSKYFEEDGETACTORFEEDS_H
#define APPBSKYFEEDGETACTORFEEDS_H

#include "atprotocol/app/bsky/feed/appbskyfeedgetfeedgenerators.h"

namespace AtProtocolInterface {

class AppBskyFeedGetActorFeeds : public AppBskyFeedGetFeedGenerators
{
public:
    explicit AppBskyFeedGetActorFeeds(QObject *parent = nullptr);

    void getActorFeeds(const QString &actor, const qint64 limit, const QString &cursor);
};

}

#endif // APPBSKYFEEDGETACTORFEEDS_H
