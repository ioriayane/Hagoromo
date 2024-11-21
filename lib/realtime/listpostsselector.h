#ifndef LISTPOSTSSELECTOR_H
#define LISTPOSTSSELECTOR_H

#include "abstractpostselector.h"

namespace RealtimeFeed {

class ListPostsSelector : public AbstractPostSelector
{
    Q_OBJECT
public:
    explicit ListPostsSelector(QObject *parent = nullptr);

    virtual bool judge(const QJsonObject &object);
    virtual bool validate() const;
    virtual QStringList canContain() const;
    virtual void setFollowing(const QList<UserInfo> &following) { Q_UNUSED(following) }
    virtual void setFollowers(const QList<UserInfo> &followers) { Q_UNUSED(followers) }
    virtual void setListMembers(const QList<UserInfo> &members);

private:
    QHash<QString, UserInfo> m_members;
};

}

#endif // LISTPOSTSSELECTOR_H
