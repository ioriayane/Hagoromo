#ifndef APPBSKYCONTACTGETMATCHES_H
#define APPBSKYCONTACTGETMATCHES_H

#include "atprotocol/app/bsky/graph/appbskygraphgetfollows.h"

namespace AtProtocolInterface {

class AppBskyContactGetMatches : public AppBskyGraphGetFollows
{
public:
    explicit AppBskyContactGetMatches(QObject *parent = nullptr);

    void getMatches(const int limit, const QString &cursor);
};

}

#endif // APPBSKYCONTACTGETMATCHES_H
