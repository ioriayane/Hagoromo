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
                QJsonObject block = getBlock(object, op.value("path").toString());
                if (!block.isEmpty()) {
                    QString subject = block.value("value").toObject().value("subject").toString();
                    m_following.append(subject);
                }
            } else if (action == "delete") {
            }
        }
    }

    return isTarget(object) && m_following.contains(getRepo(object));
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

}
