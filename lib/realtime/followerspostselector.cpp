#include "followerspostselector.h"

FollowersPostSelector::FollowersPostSelector(QObject *parent) : AbstractPostSelector { parent } { }

bool FollowersPostSelector::judge(const QJsonObject &object)
{
    // 他人の自分をフォローするデータのとき追加する
    // deleteのときは消す
    return isTarget(object) && m_followers.contains(getRepo(object));
}

void FollowersPostSelector::setFollowers(const QStringList &followers)
{
    AbstractPostSelector::setFollowers(followers);
    m_followers = followers;
}

QString FollowersPostSelector::toString()
{
    return "{\"followers\":{}}";
}
