#ifndef FOLLOWERSLISTMODEL_H
#define FOLLOWERSLISTMODEL_H

#include "followslistmodel.h"

class FollowersListModel : public FollowsListModel
{
    Q_OBJECT
public:
    explicit FollowersListModel(QObject *parent = nullptr);

public slots:
    Q_INVOKABLE void getLatest();
    Q_INVOKABLE void getNext();
};

#endif // FOLLOWERSLISTMODEL_H
