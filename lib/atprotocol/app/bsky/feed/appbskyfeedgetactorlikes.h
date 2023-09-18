#ifndef APPBSKYFEEDGETACTORLIKES_H
#define APPBSKYFEEDGETACTORLIKES_H

#include "appbskyfeedgettimeline.h"
#include "atprotocol/lexicons.h"

namespace AtProtocolInterface {

class AppBskyFeedGetActorLikes : public AppBskyFeedGetTimeline
{
public:
    explicit AppBskyFeedGetActorLikes(QObject *parent = nullptr);

    void getActorLikes(const QString &actor, const int limit, const QString &cursor);
    void getTimeline(const QString &cursor = QString()) = delete;

    const QList<AtProtocolType::AppBskyFeedDefs::FeedViewPost> *feedList() const;
};

}

#endif // APPBSKYFEEDGETACTORLIKES_H
