#include "notpostselector.h"

namespace RealtimeFeed {

NotPostSelector::NotPostSelector(QObject *parent) : AbstractPostSelector { parent }
{
    setName("not");
}

bool NotPostSelector::judge(const QJsonObject &object)
{
    if (!ready())
        return false;

    if (children().isEmpty())
        return false;

    return !(children().at(0)->judge(object));
}

}
