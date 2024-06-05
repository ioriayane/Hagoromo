#ifndef FOLLOWINGPOSTSELECTOR_H
#define FOLLOWINGPOSTSELECTOR_H

#include "abstractpostselector.h"

class FollowingPostSelector : public AbstractPostSelector
{
    Q_OBJECT
public:
    explicit FollowingPostSelector(QObject *parent = nullptr);

    virtual bool judge(const QJsonObject &object);
};

#endif // FOLLOWINGPOSTSELECTOR_H
