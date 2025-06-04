#ifndef ANYFEEDLISTMODEL_H
#define ANYFEEDLISTMODEL_H

#include "timelinelistmodel.h"

class AnyFeedListModel : public TimelineListModel
{
    Q_OBJECT

    Q_PROPERTY(QString targetDid READ targetDid WRITE setTargetDid NOTIFY targetDidChanged)
    Q_PROPERTY(QString targetHandle READ targetHandle WRITE setTargetHandle NOTIFY
                       targetHandleChanged FINAL)
    Q_PROPERTY(QString targetDisplayName READ targetDisplayName WRITE setTargetDisplayName NOTIFY
                       targetDisplayNameChanged FINAL)
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
    QString targetHandle() const;
    void setTargetHandle(const QString &newTargetHandle);
    QString targetDisplayName() const;
    void setTargetDisplayName(const QString &newTargetDisplayName);
    AnyFeedListModelFeedType feedType() const;
    void setFeedType(AnyFeedListModelFeedType newFeedType);
    QString targetServiceEndpoint() const;
    void setTargetServiceEndpoint(const QString &newTargetServiceEndpoint);

signals:
    void targetDidChanged();
    void targetHandleChanged();
    void targetDisplayNameChanged();
    void feedTypeChanged();
    void targetServiceEndpointChanged();

protected:
    virtual void finishedDisplayingQueuedPosts();

private:
    QStringList m_cueGetPost; // uri
    QHash<QString, AtProtocolType::ComAtprotoRepoListRecords::Record> m_recordHash;
    QHash<QString, AtProtocolType::ComAtprotoRepoStrongRef::Main>
            m_repostRefHash; // QHash<post cid, repost uri/cid>

    QString m_targetDid;
    QString m_targetHandle;
    QString m_targetDisplayName;
    AnyFeedListModelFeedType m_feedType;
    QString m_targetServiceEndpoint;

    void getPosts();

    template<typename T>
    QString appendGetPostCue(const QVariant &record);
    template<typename T>
    QString getIndexedAt(const QVariant &record);
};

#endif // ANYFEEDLISTMODEL_H
