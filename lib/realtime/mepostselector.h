#ifndef MEPOSTSELECTOR_H
#define MEPOSTSELECTOR_H

#include "abstractpostselector.h"

namespace RealtimeFeed {

class MePostSelector : public AbstractPostSelector
{
    Q_OBJECT
public:
    explicit MePostSelector(QObject *parent = nullptr);

    virtual bool judge(const QJsonObject &object);
    virtual bool validate() const;
    virtual QStringList canContain() const;
};

}

#endif // MEPOSTSELECTOR_H
