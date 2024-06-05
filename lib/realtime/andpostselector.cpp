#include "andpostselector.h"

AndPostSelector::AndPostSelector(QObject *parent) : AbstractPostSelector { parent } { }

bool AndPostSelector::judge(const QJsonObject &object)
{
    for (auto child : children()) {
        if (!child->judge(object))
            return false;
    }
    return true;
}
