#ifndef LISTPOSTSSELECTOR_H
#define LISTPOSTSSELECTOR_H

#include "abstractpostselector.h"

namespace RealtimeFeed {

class ListPostsSelector : public AbstractPostSelector
{
    Q_OBJECT
public:
    explicit ListPostsSelector(QObject *parent = nullptr);

    virtual bool judge(const QJsonObject &object);
    virtual bool validate() const;
    virtual QStringList canContain() const;
};

}

#endif // LISTPOSTSSELECTOR_H
