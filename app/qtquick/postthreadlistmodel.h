#ifndef POSTTHREADLISTMODEL_H
#define POSTTHREADLISTMODEL_H

#include "atprotocol/lexicons.h"
#include "timelinelistmodel.h"
#include <QObject>

class PostThreadListModel : public TimelineListModel
{
    Q_OBJECT

    Q_PROPERTY(QString postThreadUri READ postThreadUri WRITE setPostThreadUri NOTIFY
                       postThreadUriChanged)
public:
    explicit PostThreadListModel(QObject *parent = nullptr);

    Q_INVOKABLE void getLatest();

    QString postThreadUri() const;
    void setPostThreadUri(const QString &newPostThreadUri);

signals:

    void postThreadUriChanged();

protected:
    virtual void finishedDisplayingQueuedPosts();

private:
    QString m_postThreadUri;

    void copyFrom(const AtProtocolType::AppBskyFeedDefs::ThreadViewPost *thread_view_post,
                  const int type);
};

#endif // POSTTHREADLISTMODEL_H
