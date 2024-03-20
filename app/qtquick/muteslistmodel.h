#ifndef MUTESLISTMODEL_H
#define MUTESLISTMODEL_H

#include "followslistmodel.h"

class MutesListModel : public FollowsListModel
{
    Q_OBJECT
public:
    explicit MutesListModel(QObject *parent = nullptr);

public slots:
    Q_INVOKABLE bool getLatest();
    Q_INVOKABLE bool getNext();
};

#endif // MUTESLISTMODEL_H
