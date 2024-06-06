#include "andpostselector.h"

namespace RealtimeFeed {

AndPostSelector::AndPostSelector(QObject *parent) : AbstractPostSelector { parent } { }

bool AndPostSelector::judge(const QJsonObject &object)
{
    for (auto child : children()) {
        if (!child->judge(object))
            return false;
    }
    return true;
}

QString AndPostSelector::toString()
{
    QString ret = "{\"and\":[";
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
