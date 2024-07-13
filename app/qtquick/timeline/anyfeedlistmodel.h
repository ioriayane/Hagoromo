#ifndef ANYFEEDLISTMODEL_H
#define ANYFEEDLISTMODEL_H

#include "timelinelistmodel.h"

class AnyFeedListModel : public TimelineListModel
{
    Q_OBJECT

    Q_PROPERTY(QString targetDid READ targetDid WRITE setTargetDid NOTIFY targetDidChanged)
    Q_PROPERTY(AnyFeedListModelFeedType feedType READ feedType WRITE setFeedType NOTIFY
                       feedTypeChanged)
    Q_PROPERTY(QString targetServiceEndpoint READ targetServiceEndpoint WRITE
                       setTargetServiceEndpoint NOTIFY targetServiceEndpointChanged FINAL)

public:
    explicit AnyFeedListModel(QObject *parent = nullptr);

    enum AnyFeedListModelFeedType { LikeFeedType = 0, RepostFeedType = 1 };
    Q_ENUM(AnyFeedListModelFeedType)

    Q_INVOKABLE bool getLatest();
    Q_INVOKABLE bool getNext();

    QString targetDid() const;
    void setTargetDid(const QString &newTargetDid);
    AnyFeedListModelFeedType feedType() const;
    void setFeedType(AnyFeedListModelFeedType newFeedType);
    QString targetServiceEndpoint() const;
    void setTargetServiceEndpoint(const QString &newTargetServiceEndpoint);

signals:
    void targetDidChanged();
    void feedTypeChanged();
    void targetServiceEndpointChanged();

protected:
    virtual void finishedDisplayingQueuedPosts();

private:
    QStringList m_cueGetPost; // uri
    QHash<QString, AtProtocolType::ComAtprotoRepoListRecords::Record> m_recordHash;

    QString m_targetDid;
    AnyFeedListModelFeedType m_feedType;
    QString m_targetServiceEndpoint;

    void getPosts();

    template<typename T>
    QString appendGetPostCue(const QVariant &record);
    template<typename T>
    QString getIndexedAt(const QVariant &record);
};

#endif // ANYFEEDLISTMODEL_H
