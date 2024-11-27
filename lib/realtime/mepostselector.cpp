#include "mepostselector.h"

namespace RealtimeFeed {

MePostSelector::MePostSelector(QObject *parent) : AbstractPostSelector { parent }
{
    setType("me");
    setDisplayType("Me");
}

bool MePostSelector::judge(const QJsonObject &object)
{
    return (ready() && isTarget(object) && isMy(object) && matchImageCondition(object)
            && matchMovieCondition(object));
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
