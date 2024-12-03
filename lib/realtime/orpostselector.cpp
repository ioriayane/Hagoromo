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

    // フォローやリストの追加・削除の処理でひととおりjudgeを呼ぶ必要ある
    int count = 0;
    for (auto child : children()) {
        if (child->judge(object))
            count++;
    }
    return (count > 0);
}

bool OrPostSelector::validate() const
{
    if (!AbstractPostSelector::validate()) {
        return false;
    }
    return !children().isEmpty();
}
}
