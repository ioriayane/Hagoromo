#ifndef APPBSKYFEEDGETLIKES_H
#define APPBSKYFEEDGETLIKES_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyFeedGetLikes : public AccessAtProtocol
{
public:
    explicit AppBskyFeedGetLikes(QObject *parent = nullptr);

    void getLikes(const QString &uri, const QString &cid, const qint64 limit,
                  const QString &cursor);

    const QString &uri() const;
    const QString &cid() const;
    const QList<AtProtocolType::AppBskyFeedGetLikes::Like> &likesList() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QString m_uri;
    QString m_cid;
    QList<AtProtocolType::AppBskyFeedGetLikes::Like> m_likesList;
};

}

#endif // APPBSKYFEEDGETLIKES_H
