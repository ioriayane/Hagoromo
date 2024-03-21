#ifndef BLOCKSLISTMODEL_H
#define BLOCKSLISTMODEL_H

#include "followslistmodel.h"

class BlocksListModel : public FollowsListModel
{
    Q_OBJECT
public:
    explicit BlocksListModel(QObject *parent = nullptr);

public slots:
    Q_INVOKABLE bool getLatest();
    Q_INVOKABLE bool getNext();
};

#endif // BLOCKSLISTMODEL_H
