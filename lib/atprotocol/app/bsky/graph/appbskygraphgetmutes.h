#ifndef APPBSKYGRAPHGETMUTES_H
#define APPBSKYGRAPHGETMUTES_H

#include "appbskygraphgetfollows.h"

namespace AtProtocolInterface {

class AppBskyGraphGetMutes : public AppBskyGraphGetFollows
{
public:
    explicit AppBskyGraphGetMutes(QObject *parent = nullptr);

    void getFollows(const QString &actor, const int limit, const QString &cursor) = delete;
    void getMutes(const int limit, const QString &cursor);
};

}

#endif // APPBSKYGRAPHGETMUTES_H
