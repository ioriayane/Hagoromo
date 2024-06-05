#include "followerpostselector.h"

FollowerPostSelector::FollowerPostSelector(QObject *parent) : AbstractPostSelector { parent } { }

bool FollowerPostSelector::judge(const QJsonObject &object)
{
    return (object.value("follower").toInt(0) == 1);
}
