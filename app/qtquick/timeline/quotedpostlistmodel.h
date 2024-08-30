#ifndef QUOTEDPOSTLISTMODEL_H
#define QUOTEDPOSTLISTMODEL_H

#include "timelinelistmodel.h"
#include <QObject>

class QuotedPostListModel : public TimelineListModel
{
    Q_OBJECT

    Q_PROPERTY(QString postUri READ postUri WRITE setPostUri NOTIFY postUriChanged FINAL)
public:
    explicit QuotedPostListModel(QObject *parent = nullptr);

    Q_INVOKABLE bool getLatest();
    Q_INVOKABLE bool getNext();

    QString postUri() const;
    void setPostUri(const QString &newPostUri);

signals:
    void postUriChanged();

protected:
    virtual void finishedDisplayingQueuedPosts();

private:
    QString m_postUri;
};

#endif // QUOTEDPOSTLISTMODEL_H
