#ifndef ANDPOSTSELECTOR_H
#define ANDPOSTSELECTOR_H

#include "abstractpostselector.h"

namespace RealtimeFeed {

class AndPostSelector : public AbstractPostSelector
{
    Q_OBJECT
public:
    explicit AndPostSelector(QObject *parent = nullptr);

    virtual bool judge(const QJsonObject &object);
};

}

#endif // ANDPOSTSELECTOR_H
