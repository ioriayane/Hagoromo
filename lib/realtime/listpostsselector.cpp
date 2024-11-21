#include "listpostsselector.h"

namespace RealtimeFeed {

ListPostsSelector::ListPostsSelector(QObject *parent) : AbstractPostSelector { parent }
{
    setType("list");
    setDisplayType("List");
}

bool RealtimeFeed::ListPostsSelector::judge(const QJsonObject &object)
{
    if (!ready())
        return false;

    return false;
}

bool RealtimeFeed::ListPostsSelector::validate() const
{
    return listUri().startsWith("at://");
}

QStringList RealtimeFeed::ListPostsSelector::canContain() const
{
    return QStringList();
}

}
