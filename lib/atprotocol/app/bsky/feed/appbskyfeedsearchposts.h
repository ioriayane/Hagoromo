#ifndef APPBSKYFEEDSEARCHPOSTS_H
#define APPBSKYFEEDSEARCHPOSTS_H

#include "atprotocol/app/bsky/feed/appbskyfeedgetposts.h"

namespace AtProtocolInterface {

class AppBskyFeedSearchPosts : public AppBskyFeedGetPosts
{
public:
    explicit AppBskyFeedSearchPosts(QObject *parent = nullptr);

    void searchPosts(const QString &q, const int limit, const QString &cursor);
};

}

#endif // APPBSKYFEEDSEARCHPOSTS_H
