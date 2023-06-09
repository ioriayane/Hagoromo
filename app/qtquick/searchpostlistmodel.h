#ifndef SEARCHPOSTLISTMODEL_H
#define SEARCHPOSTLISTMODEL_H

#include "timelinelistmodel.h"

class SearchPostListModel : public TimelineListModel
{
    Q_OBJECT

    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
public:
    explicit SearchPostListModel(QObject *parent = nullptr);

    Q_INVOKABLE void getLatest();

    QString text() const;
    void setText(const QString &newText);

signals:
    void textChanged();

protected:
    virtual void finishedDisplayingQueuedPosts();

private:
    QStringList m_cueGetPost; // uri

    void getPosts();
    QString m_text;
};

#endif // SEARCHPOSTLISTMODEL_H
