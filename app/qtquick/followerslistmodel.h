#ifndef FOLLOWERSLISTMODEL_H
#define FOLLOWERSLISTMODEL_H

#include "followslistmodel.h"

class FollowersListModel : public FollowsListModel
{
    Q_OBJECT
public:
    explicit FollowersListModel(QObject *parent = nullptr);

public slots:
    Q_INVOKABLE bool getLatest();
    Q_INVOKABLE bool getNext();
};

#endif // FOLLOWERSLISTMODEL_H
