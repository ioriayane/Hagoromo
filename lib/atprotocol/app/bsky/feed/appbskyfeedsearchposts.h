#ifndef APPBSKYFEEDSEARCHPOSTS_H
#define APPBSKYFEEDSEARCHPOSTS_H

#include "atprotocol/app/bsky/feed/appbskyfeedgetposts.h"

namespace AtProtocolInterface {

class AppBskyFeedSearchPosts : public AppBskyFeedGetPosts
{
public:
    explicit AppBskyFeedSearchPosts(QObject *parent = nullptr);

    void searchPosts(const QString &q, const QString &sort, const QString &since,
                     const QString &until, const QString &mentions, const QString &author,
                     const QString &lang, const QString &domain, const QString &url,
                     const QList<QString> &tag, const int limit, const QString &cursor);
};

}

#endif // APPBSKYFEEDSEARCHPOSTS_H
