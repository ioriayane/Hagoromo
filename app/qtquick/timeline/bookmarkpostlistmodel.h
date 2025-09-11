#ifndef BOOKMARKPOSTLISTMODEL_H
#define BOOKMARKPOSTLISTMODEL_H

#include "timelinelistmodel.h"

class BookmarkPostListModel : public TimelineListModel
{
    Q_OBJECT
public:
    explicit BookmarkPostListModel(QObject *parent = nullptr);

    Q_INVOKABLE bool getLatest();
    Q_INVOKABLE bool getNext();

protected:
    virtual void finishedDisplayingQueuedPosts();
};

#endif // BOOKMARKPOSTLISTMODEL_H
