#include "notpostselector.h"

namespace RealtimeFeed {

NotPostSelector::NotPostSelector(QObject *parent) : AbstractPostSelector { parent }
{
    setType("not");
    setDisplayType("NOT");
}

bool NotPostSelector::judge(const QJsonObject &object)
{
    if (!ready())
        return false;

    if (children().isEmpty())
        return false;

    if (!isTarget(object))
        return false;

    return !(children().at(0)->judge(object));
}

QStringList NotPostSelector::canContain() const
{
    if (children().length() == 0) {
        return AbstractPostSelector::canContain();
    } else {
        return QStringList();
    }
}

bool NotPostSelector::validate() const
{
    if (!AbstractPostSelector::validate()) {
        return false;
    }
    return !children().isEmpty();
}
}
