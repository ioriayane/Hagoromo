#ifndef APPBSKYGRAPHGETLISTMUTES_H
#define APPBSKYGRAPHGETLISTMUTES_H

#include "atprotocol/app/bsky/graph/appbskygraphgetlists.h"

namespace AtProtocolInterface {

class AppBskyGraphGetListMutes : public AppBskyGraphGetLists
{
public:
    explicit AppBskyGraphGetListMutes(QObject *parent = nullptr);

    void getListMutes(const qint64 limit, const QString &cursor);
};

}

#endif // APPBSKYGRAPHGETLISTMUTES_H
