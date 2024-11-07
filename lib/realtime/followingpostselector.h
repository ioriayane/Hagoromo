#ifndef FOLLOWINGPOSTSELECTOR_H
#define FOLLOWINGPOSTSELECTOR_H

#include "abstractpostselector.h"

namespace RealtimeFeed {

class FollowingPostSelector : public AbstractPostSelector
{
    Q_OBJECT
public:
    explicit FollowingPostSelector(QObject *parent = nullptr);

    virtual bool judge(const QJsonObject &object);
    virtual QStringList canContain() const;
    virtual void setFollowing(const QList<UserInfo> &following);
    virtual void setFollowers(const QList<UserInfo> &followers) { Q_UNUSED(followers) }
    virtual UserInfo getUser(const QString &did) const;

    virtual bool needFollowing() const;

private:
    QHash<QString, UserInfo> m_following;
};

}

#endif // FOLLOWINGPOSTSELECTOR_H
