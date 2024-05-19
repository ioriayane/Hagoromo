#ifndef APPBSKYFEEDGETPOSTS_H
#define APPBSKYFEEDGETPOSTS_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyFeedGetPosts : public AccessAtProtocol
{
public:
    explicit AppBskyFeedGetPosts(QObject *parent = nullptr);

    void getPosts(const QList<QString> &uris);

    const QList<AtProtocolType::AppBskyFeedDefs::PostView> &postsPostViewList() const;

protected:
    QString m_listKey;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::AppBskyFeedDefs::PostView> m_postsPostViewList;
};

}

#endif // APPBSKYFEEDGETPOSTS_H
