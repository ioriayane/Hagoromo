#include "followerspostselector.h"

namespace RealtimeFeed {

FollowersPostSelector::FollowersPostSelector(QObject *parent) : AbstractPostSelector { parent }
{
    setType("followers");
    setDisplayType("Followers");
}

bool FollowersPostSelector::judge(const QJsonObject &object)
{
    if (!ready())
        return false;

    // 他人の自分をフォローするデータのとき追加する
    // deleteのときは消す
    if (!isMy(object)) {
        QJsonObject op = getOperation(object, "app.bsky.graph.follow");
        if (!op.isEmpty()) {
            QString action = op.value("action").toString();
            if (action == "create") {
                QString path = op.value("path").toString();
                QJsonObject block = getBlock(object, path);
                if (!block.isEmpty()) {
                    QString subject = block.value("value").toObject().value("subject").toString();
                    if (subject == did()) {
                        UserInfo user;
                        user.did = getRepo(object);
                        user.rkey = extractRkey(path);
                        m_followers[user.did] = user;
                    }
                }
            } else if (action == "delete") {
                QString rkey = extractRkey(op.value("path").toString());
                if (!rkey.isEmpty()) {
                    QString user_did = getRepo(object);
                    for (const auto &user : qAsConst(m_followers)) {
                        if (user.rkey == rkey && user.did == user_did) {
                            m_followers.remove(user.did);
                            break;
                        }
                    }
                }
            }
        }
    }
    return isTarget(object) && m_followers.contains(getRepo(object));
}

QStringList FollowersPostSelector::canContain() const
{
    return QStringList();
}

void FollowersPostSelector::setFollowers(const QList<UserInfo> &followers)
{
    AbstractPostSelector::setFollowers(followers);
    for (const auto &user : followers) {
        m_followers[user.did] = user;
    }
}

UserInfo FollowersPostSelector::getUser(const QString &did) const
{
    if (m_followers.contains(did)) {
        return m_followers.value(did);
    } else {
        return AbstractPostSelector::getUser(did);
    }
}

bool FollowersPostSelector::needFollowers() const
{
    return true;
}
}
