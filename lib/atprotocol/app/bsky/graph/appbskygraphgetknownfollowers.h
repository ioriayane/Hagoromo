#ifndef APPBSKYGRAPHGETKNOWNFOLLOWERS_H
#define APPBSKYGRAPHGETKNOWNFOLLOWERS_H

#include "atprotocol/app/bsky/graph/appbskygraphgetfollows.h"

namespace AtProtocolInterface {

class AppBskyGraphGetKnownFollowers : public AppBskyGraphGetFollows
{
public:
    explicit AppBskyGraphGetKnownFollowers(QObject *parent = nullptr);

    void getKnownFollowers(const QString &actor, const qint64 limit, const QString &cursor);
};

}

#endif // APPBSKYGRAPHGETKNOWNFOLLOWERS_H
