#ifndef XORPOSTSELECTOR_H
#define XORPOSTSELECTOR_H

#include "abstractpostselector.h"

namespace RealtimeFeed {

class XorPostSelector : public AbstractPostSelector
{
    Q_OBJECT
public:
    explicit XorPostSelector(QObject *parent = nullptr);

    virtual bool judge(const QJsonObject &object);
};

}

#endif // XORPOSTSELECTOR_H
