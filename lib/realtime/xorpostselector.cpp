#include "xorpostselector.h"

namespace RealtimeFeed {

XorPostSelector::XorPostSelector(QObject *parent) : AbstractPostSelector { parent } { }

bool XorPostSelector::judge(const QJsonObject &object)
{
    int count = 0;
    for (auto child : children()) {
        if (child->judge(object))
            count++;
    }
    return (count % 2 == 1);
}

QString XorPostSelector::toString()
{
    QString ret = "{\"xor\":[";
    int i = 0;
    for (auto child : children()) {
        if (i > 0) {
            ret += ",";
        }
        ret += child->toString();
        i++;
    }
    ret += "]}";
    return ret;
}

}
