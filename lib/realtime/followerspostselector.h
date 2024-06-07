#ifndef FOLLOWERSPOSTSELECTOR_H
#define FOLLOWERSPOSTSELECTOR_H

#include "abstractpostselector.h"

namespace RealtimeFeed {

class FollowersPostSelector : public AbstractPostSelector
{
    Q_OBJECT
public:
    explicit FollowersPostSelector(QObject *parent = nullptr);

    virtual bool judge(const QJsonObject &object);
    virtual void setFollowing(const QList<UserInfo> &following) { Q_UNUSED(following) }
    virtual void setFollowers(const QList<UserInfo> &followers);
    virtual QString toString();

    virtual bool needFollowers() const;

private:
    QHash<QString, UserInfo> m_followers;
};

}

#endif // FOLLOWERSPOSTSELECTOR_H
