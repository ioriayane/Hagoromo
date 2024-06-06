#include "followingpostselector.h"

namespace RealtimeFeed {

FollowingPostSelector::FollowingPostSelector(QObject *parent) : AbstractPostSelector { parent } { }

bool FollowingPostSelector::judge(const QJsonObject &object)
{
    // 自分のフォローのデータのときリストに追加
    // deleteのときは消す
    if (isMy(object)) {
        QJsonObject op = getOperation(object, "app.bsky.graph.follow");
        if (!op.isEmpty()) {
            QString action = op.value("action").toString();
            if (action == "create") {
                QString path = op.value("path").toString();
                QJsonObject block = getBlock(object, path);
                if (!block.isEmpty()) {
                    QString subject = block.value("value").toObject().value("subject").toString();
                    UserInfo user;
                    user.did = subject;
                    user.rkey = extractRkey(path);
                    m_following[subject] = user;
                }
            } else if (action == "delete") {
            }
        }
    }

    return isTarget(object) && m_following.contains(getRepo(object));
}

void FollowingPostSelector::setFollowing(const QList<UserInfo> &following)
{
    AbstractPostSelector::setFollowing(following);
    for (const auto &user : following) {
        m_following[user.did] = user;
    }
}

QString FollowingPostSelector::toString()
{
    return "{\"following\":{}}";
}

}
