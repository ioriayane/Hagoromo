#include "andpostselector.h"

namespace RealtimeFeed {

AndPostSelector::AndPostSelector(QObject *parent) : AbstractPostSelector { parent }
{
    setType("and");
    setDisplayType("AND");
    setIsArray(true);
}

bool AndPostSelector::judge(const QJsonObject &object)
{
    if (!ready())
        return false;

    // フォローやリストの追加・削除の処理でひととおりjudgeを呼ぶ必要ある
    int count = 0;
    for (auto child : children()) {
        if (child->judge(object))
            count++;
    }
    return (count == children().count());
}

bool AndPostSelector::validate() const
{
    if (!AbstractPostSelector::validate()) {
        return false;
    }
    return !children().isEmpty();
}
}
