#include "followingpostselector.h"

FollowingPostSelector::FollowingPostSelector(QObject *parent) : AbstractPostSelector { parent } { }

bool FollowingPostSelector::judge(const QJsonObject &object)
{
    return (object.value("following").toInt(0) == 1);
}
