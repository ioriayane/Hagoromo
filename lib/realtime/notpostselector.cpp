#include "notpostselector.h"

namespace RealtimeFeed {

NotPostSelector::NotPostSelector(QObject *parent) : AbstractPostSelector { parent } { }

bool NotPostSelector::judge(const QJsonObject &object)
{
    if (children().isEmpty())
        return false;

    return !(children().at(0)->judge(object));
}

QString NotPostSelector::toString()
{
    QString ret = "{\"not\":";

    if (!children().isEmpty())
        ret += children().at(0)->toString();

    ret += "}";
    return ret;
}

}
