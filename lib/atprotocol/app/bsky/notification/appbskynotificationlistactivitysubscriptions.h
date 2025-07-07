#ifndef APPBSKYNOTIFICATIONLISTACTIVITYSUBSCRIPTIONS_H
#define APPBSKYNOTIFICATIONLISTACTIVITYSUBSCRIPTIONS_H

#include "atprotocol/app/bsky/graph/appbskygraphgetfollows.h"

namespace AtProtocolInterface {

class AppBskyNotificationListActivitySubscriptions : public AppBskyGraphGetFollows
{
public:
    explicit AppBskyNotificationListActivitySubscriptions(QObject *parent = nullptr);

    void listActivitySubscriptions(const qint64 limit, const QString &cursor);
};

}

#endif // APPBSKYNOTIFICATIONLISTACTIVITYSUBSCRIPTIONS_H
