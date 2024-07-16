#ifndef LISTMUTESLISTMODEL_H
#define LISTMUTESLISTMODEL_H

#include "listslistmodel.h"

class ListMutesListModel : public ListsListModel
{
    Q_OBJECT
public:
    explicit ListMutesListModel(QObject *parent = nullptr);

public slots:
    Q_INVOKABLE bool getLatest();
    Q_INVOKABLE bool getNext();
};

#endif // LISTMUTESLISTMODEL_H
