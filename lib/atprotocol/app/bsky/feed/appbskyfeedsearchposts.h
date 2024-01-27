#ifndef APPBSKYFEEDSEARCHPOSTS_H
#define APPBSKYFEEDSEARCHPOSTS_H

#include "appbskyfeedgettimeline.h"

namespace AtProtocolInterface {

class AppBskyFeedSearchPosts : public AppBskyFeedGetTimeline
{
public:
    explicit AppBskyFeedSearchPosts(QObject *parent = nullptr);

    void searchPosts(const QString &q, const int limit, const QString &cursor);

private:
    virtual bool parseJson(bool success, const QString reply_json);
};

}

#endif // APPBSKYFEEDSEARCHPOSTS_H
