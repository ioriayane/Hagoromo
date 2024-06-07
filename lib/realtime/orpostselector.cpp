#include "orpostselector.h"

namespace RealtimeFeed {

OrPostSelector::OrPostSelector(QObject *parent) : AbstractPostSelector { parent }
{
    setName("or");
    setIsArray(true);
}

bool OrPostSelector::judge(const QJsonObject &object)
{
    for (auto child : children()) {
        if (child->judge(object))
            return true;
    }
    return false;
}

}
