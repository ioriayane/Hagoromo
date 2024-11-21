#include "listpostsselector.h"

namespace RealtimeFeed {

ListPostsSelector::ListPostsSelector(QObject *parent) : AbstractPostSelector { parent }
{
    setType("list");
    setDisplayType("List");
}

bool RealtimeFeed::ListPostsSelector::judge(const QJsonObject &object)
{
    if (!ready())
        return false;

    return isTarget(object) && m_members.contains(getRepo(object));
}

bool RealtimeFeed::ListPostsSelector::validate() const
{
    return listUri().startsWith("at://");
}

QStringList RealtimeFeed::ListPostsSelector::canContain() const
{
    return QStringList();
}

void ListPostsSelector::setListMembers(const QList<UserInfo> &members)
{
    AbstractPostSelector::setListMembers(members);
    for (const auto &user : members) {
        m_members[user.did] = user;
    }
}
}
