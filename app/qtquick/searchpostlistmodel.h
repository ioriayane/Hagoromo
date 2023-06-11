#ifndef SEARCHPOSTLISTMODEL_H
#define SEARCHPOSTLISTMODEL_H

#include "timelinelistmodel.h"

class SearchPostListModel : public TimelineListModel
{
    Q_OBJECT

    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QString searchService READ searchService WRITE setSearchService NOTIFY
                       searchServiceChanged)

public:
    explicit SearchPostListModel(QObject *parent = nullptr);

    Q_INVOKABLE void getLatest();

    QString text() const;
    void setText(const QString &newText);
    QString searchService() const;
    void setSearchService(const QString &newSearchService);

signals:
    void textChanged();
    void searchServiceChanged();

protected:
    virtual void finishedDisplayingQueuedPosts();

private:
    QStringList m_cueGetPost; // uri

    void getPosts();
    QString m_text;
    QString m_searchService;
};

#endif // SEARCHPOSTLISTMODEL_H
