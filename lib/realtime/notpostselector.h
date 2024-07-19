#ifndef NOTPOSTSELECTOR_H
#define NOTPOSTSELECTOR_H

#include "abstractpostselector.h"

namespace RealtimeFeed {

class NotPostSelector : public AbstractPostSelector
{
    Q_OBJECT
public:
    explicit NotPostSelector(QObject *parent = nullptr);

    virtual bool judge(const QJsonObject &object);
};

}

#endif // NOTPOSTSELECTOR_H
