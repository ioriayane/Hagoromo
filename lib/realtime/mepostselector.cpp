#include "mepostselector.h"

MePostSelector::MePostSelector(QObject *parent) : AbstractPostSelector { parent } { }

bool MePostSelector::judge(const QJsonObject &object)
{
    return (getRepo(object) == did());
}

QString MePostSelector::toString()
{
    return "{\"me\":{}}";
}
