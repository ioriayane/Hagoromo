#ifndef APPBSKYFEEDGETPOSTS_H
#define APPBSKYFEEDGETPOSTS_H

#include "atprotocol/accessatprotocol.h"
#include "atprotocol/lexicons.h"

namespace AtProtocolInterface {

class AppBskyFeedGetPosts : public AccessAtProtocol
{
public:
    explicit AppBskyFeedGetPosts(QObject *parent = nullptr);

    void getPosts(const QList<QString> &uris);

    const QList<AtProtocolType::AppBskyFeedDefs::PostView> *postList() const;

private:
    virtual void parseJson(const QString reply_json);

    QList<AtProtocolType::AppBskyFeedDefs::PostView> m_postList;
};

}

#endif // APPBSKYFEEDGETPOSTS_H
