#include "followingpostselector.h"

namespace RealtimeFeed {

FollowingPostSelector::FollowingPostSelector(QObject *parent) : AbstractPostSelector { parent }
{
    setType("following");
    setDisplayType("Following");
}

bool FollowingPostSelector::judge(const QJsonObject &object)
{
    if (!ready())
        return false;

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
                    m_following[user.did] = user;
                }
            } else if (action == "delete") {
                QString rkey = extractRkey(op.value("path").toString());
                if (!rkey.isEmpty()) {
                    for (const auto &user : qAsConst(m_following)) {
                        if (user.rkey == rkey) {
                            m_following.remove(user.did);
                            break;
                        }
                    }
                }
            }
        }
    }

    return isTarget(object) && m_following.contains(getRepo(object)) && matchImageCondition(object)
            && matchMovieCondition(object) && matchQuoteCondition(object)
            && matchRepostCondition(object);
}

bool FollowingPostSelector::validate() const
{
    return true;
}

QStringList FollowingPostSelector::canContain() const
{
    return QStringList();
}

void FollowingPostSelector::setFollowing(const QList<UserInfo> &following)
{
    AbstractPostSelector::setFollowing(following);
    for (const auto &user : following) {
        m_following[user.did] = user;
    }
}

UserInfo FollowingPostSelector::getUser(const QString &did) const
{
    if (m_following.contains(did)) {
        return m_following.value(did);
    } else {
        return AbstractPostSelector::getUser(did);
    }
}

bool FollowingPostSelector::needFollowing() const
{
    return true;
}
}
