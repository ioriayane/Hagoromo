#include "followingpostselector.h"

FollowingPostSelector::FollowingPostSelector(QObject *parent) : AbstractPostSelector { parent } { }

bool FollowingPostSelector::judge(const QJsonObject &object)
{
    // 自分のフォローのデータのときリストに追加

    return m_following.contains(getRepo(object));
}

void FollowingPostSelector::setFollowing(const QStringList &following)
{
    AbstractPostSelector::setFollowing(following);
    m_following = following;
}

QString FollowingPostSelector::toString()
{
    return "{\"following\":{}}";
}
