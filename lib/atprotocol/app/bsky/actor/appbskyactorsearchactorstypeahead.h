#ifndef APPBSKYACTORSEARCHACTORSTYPEAHEAD_H
#define APPBSKYACTORSEARCHACTORSTYPEAHEAD_H

#include "atprotocol/app/bsky/graph/appbskygraphgetfollows.h"

namespace AtProtocolInterface {

class AppBskyActorSearchActorsTypeahead : public AppBskyGraphGetFollows
{
public:
    explicit AppBskyActorSearchActorsTypeahead(QObject *parent = nullptr);

    void searchActorsTypeahead(const QString &q, const int limit);
};

}

#endif // APPBSKYACTORSEARCHACTORSTYPEAHEAD_H
