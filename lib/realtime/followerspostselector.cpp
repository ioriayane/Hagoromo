#include "followerspostselector.h"

namespace RealtimeFeed {

FollowersPostSelector::FollowersPostSelector(QObject *parent) : AbstractPostSelector { parent } { }

bool FollowersPostSelector::judge(const QJsonObject &object)
{
    // 他人の自分をフォローするデータのとき追加する
    // deleteのときは消す

    return isTarget(object) && m_followers.contains(getRepo(object));
}

void FollowersPostSelector::setFollowers(const QList<UserInfo> &followers)
{
    AbstractPostSelector::setFollowers(followers);
    for (const auto &user : followers) {
        m_followers[user.did] = user;
    }
}

QString FollowersPostSelector::toString()
{
    return "{\"followers\":{}}";
}

}
