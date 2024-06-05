#ifndef MEPOSTSELECTOR_H
#define MEPOSTSELECTOR_H

#include "abstractpostselector.h"

class MePostSelector : public AbstractPostSelector
{
    Q_OBJECT
public:
    explicit MePostSelector(QObject *parent = nullptr);

    virtual bool judge(const QJsonObject &object);
};

#endif // MEPOSTSELECTOR_H
