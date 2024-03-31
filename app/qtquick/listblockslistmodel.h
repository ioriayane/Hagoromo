#ifndef LISTBLOCKSLISTMODEL_H
#define LISTBLOCKSLISTMODEL_H

#include "listslistmodel.h"

class ListBlocksListModel : public ListsListModel
{
    Q_OBJECT
public:
    explicit ListBlocksListModel(QObject *parent = nullptr);

public slots:
    Q_INVOKABLE bool getLatest();
    Q_INVOKABLE bool getNext();
};

#endif // LISTBLOCKSLISTMODEL_H
