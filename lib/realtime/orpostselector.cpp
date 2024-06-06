#include "orpostselector.h"

namespace RealtimeFeed {

OrPostSelector::OrPostSelector(QObject *parent) : AbstractPostSelector { parent } { }

bool OrPostSelector::judge(const QJsonObject &object)
{
    for (auto child : children()) {
        if (child->judge(object))
            return true;
    }
    return false;
}

QString OrPostSelector::toString()
{
    QString ret = "{\"or\":[";
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
