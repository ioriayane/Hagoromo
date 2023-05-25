#ifndef APPBSKYGRAPHGETFOLLOWERS_H
#define APPBSKYGRAPHGETFOLLOWERS_H

#include "atprotocol/accessatprotocol.h"
#include "atprotocol/lexicons.h"
#include "appbskygraphgetfollows.h"

namespace AtProtocolInterface {

class AppBskyGraphGetFollowers : public AppBskyGraphGetFollows
{
public:
    explicit AppBskyGraphGetFollowers(QObject *parent = nullptr);

    void getFollowers(const QString &actor, const int limit, const QString &cursor);

private:
    //    virtual void parseJson(const QString reply_json);
};

}

#endif // APPBSKYGRAPHGETFOLLOWERS_H
