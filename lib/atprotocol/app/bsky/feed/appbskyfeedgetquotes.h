#ifndef APPBSKYFEEDGETQUOTES_H
#define APPBSKYFEEDGETQUOTES_H

#include "atprotocol/app/bsky/feed/appbskyfeedgetposts.h"

namespace AtProtocolInterface {

class AppBskyFeedGetQuotes : public AppBskyFeedGetPosts
{
public:
    explicit AppBskyFeedGetQuotes(QObject *parent = nullptr);

    void getQuotes(const QString &uri, const QString &cid, const qint64 limit,
                   const QString &cursor);
};

}

#endif // APPBSKYFEEDGETQUOTES_H
