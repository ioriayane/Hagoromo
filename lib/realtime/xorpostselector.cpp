#include "xorpostselector.h"

namespace RealtimeFeed {

XorPostSelector::XorPostSelector(QObject *parent) : AbstractPostSelector { parent }
{
    setType("xor");
    setDisplayType("XOR");
    setIsArray(true);
}

bool XorPostSelector::judge(const QJsonObject &object)
{
    if (!ready())
        return false;

    int count = 0;
    for (auto child : children()) {
        if (child->judge(object))
            count++;
    }
    return (count % 2 == 1);
}

bool XorPostSelector::validate() const
{
    if (!AbstractPostSelector::validate()) {
        return false;
    }
    return !children().isEmpty();
}
}
