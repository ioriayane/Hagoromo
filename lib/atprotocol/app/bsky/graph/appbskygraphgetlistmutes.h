#ifndef APPBSKYGRAPHGETLISTMUTES_H
#define APPBSKYGRAPHGETLISTMUTES_H

#include "appbskygraphgetlists.h"

namespace AtProtocolInterface {

class AppBskyGraphGetListMutes : public AppBskyGraphGetLists
{
public:
    explicit AppBskyGraphGetListMutes(QObject *parent = nullptr);

    void getListMutes(const int limit, const QString &cursor);
    void getLists(const QString &actor, const int limit, const QString &cursor) = delete;
};

}

#endif // APPBSKYGRAPHGETLISTMUTES_H
