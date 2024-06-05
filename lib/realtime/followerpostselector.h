#ifndef FOLLOWERPOSTSELECTOR_H
#define FOLLOWERPOSTSELECTOR_H

#include "abstractpostselector.h"

class FollowerPostSelector : public AbstractPostSelector
{
    Q_OBJECT
public:
    explicit FollowerPostSelector(QObject *parent = nullptr);

    virtual bool judge(const QJsonObject &object);
};

#endif // FOLLOWERPOSTSELECTOR_H
