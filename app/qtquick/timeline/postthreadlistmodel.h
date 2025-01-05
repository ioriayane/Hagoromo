#ifndef POSTTHREADLISTMODEL_H
#define POSTTHREADLISTMODEL_H

#include "atprotocol/lexicons.h"
#include "timelinelistmodel.h"
#include "tools/labelprovider.h"
#include <QObject>

class PostThreadListModel : public TimelineListModel
{
    Q_OBJECT

    Q_PROPERTY(QString postThreadUri READ postThreadUri WRITE setPostThreadUri NOTIFY
                       postThreadUriChanged)
public:
    explicit PostThreadListModel(QObject *parent = nullptr);
    ~PostThreadListModel();

    Q_INVOKABLE bool getLatest();

    QString postThreadUri() const;
    void setPostThreadUri(const QString &newPostThreadUri);

signals:
    void postThreadUriChanged();

public slots:
    void finishedConnector(const QString &labeler_did);

protected:
    virtual void finishedDisplayingQueuedPosts();

private:
    QString m_postThreadCid;
    QString m_postThreadUri;
    LabelConnector m_labelConnector;

    void copyFrom(const AtProtocolType::AppBskyFeedDefs::ThreadViewPost *thread_view_post,
                  const int type);
};

#endif // POSTTHREADLISTMODEL_H
