#ifndef APPBSKYGRAPHGETBLOCKS_H
#define APPBSKYGRAPHGETBLOCKS_H

#include "appbskygraphgetfollows.h"

namespace AtProtocolInterface {

class AppBskyGraphGetBlocks : public AppBskyGraphGetFollows
{
public:
    explicit AppBskyGraphGetBlocks(QObject *parent = nullptr);

    void getFollows(const QString &actor, const int limit, const QString &cursor) = delete;
    void getBlocks(const int limit, const QString &cursor);
};

}

#endif // APPBSKYGRAPHGETBLOCKS_H
