#include "xorpostselector.h"

namespace RealtimeFeed {

XorPostSelector::XorPostSelector(QObject *parent) : AbstractPostSelector { parent }
{
    setName("xor");
    setIsArray(true);
}

bool XorPostSelector::judge(const QJsonObject &object)
{
    int count = 0;
    for (auto child : children()) {
        if (child->judge(object))
            count++;
    }
    return (count % 2 == 1);
}

}
