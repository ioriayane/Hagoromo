#ifndef APPBSKYFEEDGETLIKES_H
#define APPBSKYFEEDGETLIKES_H

#include "atprotocol/accessatprotocol.h"
#include "atprotocol/lexicons.h"

namespace AtProtocolInterface {

class AppBskyFeedGetLikes : public AccessAtProtocol
{
public:
    explicit AppBskyFeedGetLikes(QObject *parent = nullptr);

    const QList<AtProtocolType::AppBskyFeedGetLikes::Like> *likes() const;

    void getLikes(const QString &uri, const QString &cid, const int limit, const QString &cursor);

private:
    virtual void parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::AppBskyFeedGetLikes::Like> m_likes;
};

}

#endif // APPBSKYFEEDGETLIKES_H
