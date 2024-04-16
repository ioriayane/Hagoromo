#ifndef APPBSKYACTORSEARCHACTORS_H
#define APPBSKYACTORSEARCHACTORS_H

#include "atprotocol/app/bsky/graph/appbskygraphgetfollows.h"

namespace AtProtocolInterface {

class AppBskyActorSearchActors : public AppBskyGraphGetFollows
{
public:
    explicit AppBskyActorSearchActors(QObject *parent = nullptr);

    void searchActors(const QString &q, const int limit, const QString &cursor);
};

}

#endif // APPBSKYACTORSEARCHACTORS_H
