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
    virtual bool validate() const;
    virtual QStringList canContain() const;
    virtual void setFollowing(const QList<UserInfo> &following) { Q_UNUSED(following) }
    virtual void setFollowers(const QList<UserInfo> &followers);
    virtual void setListMembers(const QList<UserInfo> &members) { Q_UNUSED(members) }
    virtual UserInfo getUser(const QString &did) const;

    virtual bool needFollowers() const;

private:
    QHash<QString, UserInfo> m_followers;
};

}

#endif // FOLLOWERSPOSTSELECTOR_H
