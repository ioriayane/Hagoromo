#ifndef APPBSKYGRAPHGETLISTBLOCKS_H
#define APPBSKYGRAPHGETLISTBLOCKS_H

#include "appbskygraphgetlists.h"

namespace AtProtocolInterface {

class AppBskyGraphGetListBlocks : public AppBskyGraphGetLists
{
public:
    explicit AppBskyGraphGetListBlocks(QObject *parent = nullptr);

    void getListBlocks(const int limit, const QString &cursor);
    void getLists(const QString &actor, const int limit, const QString &cursor) = delete;
};

}

#endif // APPBSKYGRAPHGETLISTBLOCKS_H
