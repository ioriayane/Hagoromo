#ifndef APPBSKYUNSPECCEDGETPOPULARFEEDGENERATORS_H
#define APPBSKYUNSPECCEDGETPOPULARFEEDGENERATORS_H

#include "atprotocol/app/bsky/feed/appbskyfeedgetfeedgenerators.h"

namespace AtProtocolInterface {

class AppBskyUnspeccedGetPopularFeedGenerators : public AppBskyFeedGetFeedGenerators
{
public:
    explicit AppBskyUnspeccedGetPopularFeedGenerators(QObject *parent = nullptr);

    void getPopularFeedGenerators(const qint64 limit, const QString &cursor, const QString &query);
};

}

#endif // APPBSKYUNSPECCEDGETPOPULARFEEDGENERATORS_H
