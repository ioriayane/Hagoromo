#ifndef FOLLOWINGPOSTSELECTOR_H
#define FOLLOWINGPOSTSELECTOR_H

#include "abstractpostselector.h"

class FollowingPostSelector : public AbstractPostSelector
{
    Q_OBJECT
public:
    explicit FollowingPostSelector(QObject *parent = nullptr);

    virtual bool judge(const QJsonObject &object);
    virtual void setFollowing(const QStringList &following);
    virtual void setFollowers(const QStringList &followers) { Q_UNUSED(followers) }
    virtual QString toString();

private:
    QStringList m_following;
};

#endif // FOLLOWINGPOSTSELECTOR_H
