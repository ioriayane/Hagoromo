#ifndef APPBSKYGRAPHGETBLOCKS_H
#define APPBSKYGRAPHGETBLOCKS_H

#include "atprotocol/app/bsky/graph/appbskygraphgetfollows.h"

namespace AtProtocolInterface {

class AppBskyGraphGetBlocks : public AppBskyGraphGetFollows
{
public:
    explicit AppBskyGraphGetBlocks(QObject *parent = nullptr);

    void getBlocks(const qint64 limit, const QString &cursor);
};

}

#endif // APPBSKYGRAPHGETBLOCKS_H
