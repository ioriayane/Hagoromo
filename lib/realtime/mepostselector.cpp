#include "mepostselector.h"

MePostSelector::MePostSelector(QObject *parent) : AbstractPostSelector { parent } { }

bool MePostSelector::judge(const QJsonObject &object)
{
    return (object.value("did").toString() == did());
}
