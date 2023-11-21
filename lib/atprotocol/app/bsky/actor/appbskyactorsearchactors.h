#ifndef APPBSKYACTORSEARCHACTORS_H
#define APPBSKYACTORSEARCHACTORS_H

#include "atprotocol/app/bsky/graph/appbskygraphgetfollows.h"

namespace AtProtocolInterface {

class AppBskyActorSearchActors : public AppBskyGraphGetFollows
{
public:
    explicit AppBskyActorSearchActors(QObject *parent = nullptr);

    bool getFollows(const QString &actor, const int limit, const QString &cursor) = delete;
    bool getFollowers(const QString &actor, const int limit, const QString &cursor) = delete;
    bool searchActors(const QString &q, const int limit, const QString &cursor);
};

}

#endif // APPBSKYACTORSEARCHACTORS_H
