#ifndef KNOWNFOLLOWERSLISTMODEL_H
#define KNOWNFOLLOWERSLISTMODEL_H

#include "followslistmodel.h"

class KnownFollowersListModel : public FollowsListModel
{
    Q_OBJECT
public:
    explicit KnownFollowersListModel(QObject *parent = nullptr);

public slots:
    Q_INVOKABLE bool getLatest();
    Q_INVOKABLE bool getNext();
};

#endif // KNOWNFOLLOWERSLISTMODEL_H
