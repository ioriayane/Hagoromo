#ifndef FOLLOWERSPOSTSELECTOR_H
#define FOLLOWERSPOSTSELECTOR_H

#include "abstractpostselector.h"

class FollowersPostSelector : public AbstractPostSelector
{
    Q_OBJECT
public:
    explicit FollowersPostSelector(QObject *parent = nullptr);

    virtual bool judge(const QJsonObject &object);
    virtual void setFollowing(const QStringList &following) { Q_UNUSED(following) }
    virtual void setFollowers(const QStringList &followers);
    virtual QString toString();

private:
    QStringList m_followers;
};

#endif // FOLLOWERSPOSTSELECTOR_H
