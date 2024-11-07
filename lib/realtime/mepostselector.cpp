#include "mepostselector.h"

namespace RealtimeFeed {

MePostSelector::MePostSelector(QObject *parent) : AbstractPostSelector { parent }
{
    setType("me");
    setDisplayType("Me");
}

bool MePostSelector::judge(const QJsonObject &object)
{
    return (ready() && isTarget(object) && isMy(object));
}

bool MePostSelector::validate() const
{
    return true;
}

QStringList MePostSelector::canContain() const
{
    return QStringList();
}
}
