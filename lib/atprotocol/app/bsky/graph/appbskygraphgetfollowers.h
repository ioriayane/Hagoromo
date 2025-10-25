#ifndef APPBSKYGRAPHGETFOLLOWERS_H
#define APPBSKYGRAPHGETFOLLOWERS_H

#include "atprotocol/app/bsky/graph/appbskygraphgetfollows.h"

namespace AtProtocolInterface {

class AppBskyGraphGetFollowers : public AppBskyGraphGetFollows
{
public:
    explicit AppBskyGraphGetFollowers(QObject *parent = nullptr);

    void getFollowers(const QString &actor, const qint64 limit, const QString &cursor);
};

}

#endif // APPBSKYGRAPHGETFOLLOWERS_H
