#ifndef APPBSKYGRAPHGETLISTBLOCKS_H
#define APPBSKYGRAPHGETLISTBLOCKS_H

#include "atprotocol/app/bsky/graph/appbskygraphgetlists.h"

namespace AtProtocolInterface {

class AppBskyGraphGetListBlocks : public AppBskyGraphGetLists
{
public:
    explicit AppBskyGraphGetListBlocks(QObject *parent = nullptr);

    void getListBlocks(const int limit, const QString &cursor);
};

}

#endif // APPBSKYGRAPHGETLISTBLOCKS_H
