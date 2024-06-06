#include "mepostselector.h"

MePostSelector::MePostSelector(QObject *parent) : AbstractPostSelector { parent } { }

bool MePostSelector::judge(const QJsonObject &object)
{
    return (isTarget(object) && isMy(object));
}

QString MePostSelector::toString()
{
    return "{\"me\":{}}";
}
