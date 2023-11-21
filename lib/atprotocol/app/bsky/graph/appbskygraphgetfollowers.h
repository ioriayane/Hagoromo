#ifndef APPBSKYGRAPHGETFOLLOWERS_H
#define APPBSKYGRAPHGETFOLLOWERS_H

#include "appbskygraphgetfollows.h"

namespace AtProtocolInterface {

class AppBskyGraphGetFollowers : public AppBskyGraphGetFollows
{
public:
    explicit AppBskyGraphGetFollowers(QObject *parent = nullptr);

    bool getFollows(const QString &actor, const int limit, const QString &cursor) = delete;
    bool getFollowers(const QString &actor, const int limit, const QString &cursor);

private:
    //    virtual void parseJson(const QString reply_json);
};

}

#endif // APPBSKYGRAPHGETFOLLOWERS_H
