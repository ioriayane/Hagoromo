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
    virtual void setFollowing(const QList<UserInfo> &following);
    virtual void setFollowers(const QList<UserInfo> &followers) { Q_UNUSED(followers) }
    virtual QString toString();

    virtual bool needFollowing() const;

private:
    QHash<QString, UserInfo> m_following;
};

}

#endif // FOLLOWINGPOSTSELECTOR_H
