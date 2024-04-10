#ifndef APPBSKYGRAPHGETMUTES_H
#define APPBSKYGRAPHGETMUTES_H

#include "atprotocol/app/bsky/graph/appbskygraphgetfollows.h"

namespace AtProtocolInterface {

class AppBskyGraphGetMutes : public AppBskyGraphGetFollows
{
public:
    explicit AppBskyGraphGetMutes(QObject *parent = nullptr);

    void getMutes(const int limit, const QString &cursor);
};

}

#endif // APPBSKYGRAPHGETMUTES_H
