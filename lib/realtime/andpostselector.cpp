#include "andpostselector.h"

namespace RealtimeFeed {

AndPostSelector::AndPostSelector(QObject *parent) : AbstractPostSelector { parent }
{
    setName("and");
    setIsArray(true);
}

bool AndPostSelector::judge(const QJsonObject &object)
{
    if (!ready())
        return false;

    for (auto child : children()) {
        if (!child->judge(object))
            return false;
    }
    return true;
}

}
