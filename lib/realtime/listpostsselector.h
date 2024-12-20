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
    virtual void setListMembers(const QString &list_uri, const QList<UserInfo> &members);
    virtual UserInfo getUser(const QString &did) const;

    virtual bool needListMembers() const;
    virtual QStringList getListUris() const;

private:
    QHash<QString, UserInfo> m_members;
};

}

#endif // LISTPOSTSSELECTOR_H
