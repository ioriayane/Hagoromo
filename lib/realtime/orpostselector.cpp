#include "orpostselector.h"

namespace RealtimeFeed {

OrPostSelector::OrPostSelector(QObject *parent) : AbstractPostSelector { parent }
{
    setType("or");
    setDisplayType("OR");
    setIsArray(true);
}

bool OrPostSelector::judge(const QJsonObject &object)
{
    if (!ready())
        return false;

    for (auto child : children()) {
        if (child->judge(object))
            return true;
    }
    return false;
}

}
