#ifndef APPBSKYFEEDGETACTORLIKES_H
#define APPBSKYFEEDGETACTORLIKES_H

#include "atprotocol/app/bsky/feed/appbskyfeedgettimeline.h"

namespace AtProtocolInterface {

class AppBskyFeedGetActorLikes : public AppBskyFeedGetTimeline
{
public:
    explicit AppBskyFeedGetActorLikes(QObject *parent = nullptr);

    void getActorLikes(const QString &actor, const int limit, const QString &cursor);
};

}

#endif // APPBSKYFEEDGETACTORLIKES_H
