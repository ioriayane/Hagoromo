#include "mepostselector.h"

namespace RealtimeFeed {

MePostSelector::MePostSelector(QObject *parent) : AbstractPostSelector { parent }
{
    setName("me");
}

bool MePostSelector::judge(const QJsonObject &object)
{
    return (ready() && isTarget(object) && isMy(object));
}

}
