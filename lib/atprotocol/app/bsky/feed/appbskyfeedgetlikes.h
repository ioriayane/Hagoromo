#ifndef APPBSKYFEEDGETLIKES_H
#define APPBSKYFEEDGETLIKES_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyFeedGetLikes : public AccessAtProtocol
{
public:
    explicit AppBskyFeedGetLikes(QObject *parent = nullptr);

    void getLikes(const QString &uri, const QString &cid, const int limit, const QString &cursor);

    const QString &uri() const;
    const QString &cid() const;
    const QList<AtProtocolType::AppBskyFeedGetLikes::Like> &likeList() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QString m_uri;
    QString m_cid;
    QList<AtProtocolType::AppBskyFeedGetLikes::Like> m_likeList;
};

}

#endif // APPBSKYFEEDGETLIKES_H
