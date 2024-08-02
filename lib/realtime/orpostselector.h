#ifndef ORPOSTSELECTOR_H
#define ORPOSTSELECTOR_H

#include "abstractpostselector.h"

namespace RealtimeFeed {

class OrPostSelector : public AbstractPostSelector
{
    Q_OBJECT
public:
    explicit OrPostSelector(QObject *parent = nullptr);

    virtual bool judge(const QJsonObject &object);
};

}

#endif // ORPOSTSELECTOR_H
