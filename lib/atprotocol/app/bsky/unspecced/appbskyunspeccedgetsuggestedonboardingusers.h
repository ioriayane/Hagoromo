#ifndef APPBSKYUNSPECCEDGETSUGGESTEDONBOARDINGUSERS_H
#define APPBSKYUNSPECCEDGETSUGGESTEDONBOARDINGUSERS_H

#include "atprotocol/app/bsky/graph/appbskygraphgetfollows.h"

namespace AtProtocolInterface {

class AppBskyUnspeccedGetSuggestedOnboardingUsers : public AppBskyGraphGetFollows
{
public:
    explicit AppBskyUnspeccedGetSuggestedOnboardingUsers(QObject *parent = nullptr);

    void getSuggestedOnboardingUsers(const QString &category, const int limit);
};

}

#endif // APPBSKYUNSPECCEDGETSUGGESTEDONBOARDINGUSERS_H
