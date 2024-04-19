#ifndef APPBSKYFEEDGETREPOSTEDBY_H
#define APPBSKYFEEDGETREPOSTEDBY_H

#include "atprotocol/app/bsky/graph/appbskygraphgetfollows.h"

namespace AtProtocolInterface {

class AppBskyFeedGetRepostedBy : public AppBskyGraphGetFollows
{
public:
    explicit AppBskyFeedGetRepostedBy(QObject *parent = nullptr);

    void getRepostedBy(const QString &uri, const QString &cid, const int limit,
                       const QString &cursor);
};

}

#endif // APPBSKYFEEDGETREPOSTEDBY_H
