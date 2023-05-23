#ifndef ANYFEEDLISTMODEL_H
#define ANYFEEDLISTMODEL_H

#include "timelinelistmodel.h"

class AnyFeedListModel : public TimelineListModel
{
    Q_OBJECT

    Q_PROPERTY(QString targetDid READ targetDid WRITE setTargetDid NOTIFY targetDidChanged)
    Q_PROPERTY(AnyFeedListModelFeedType feedType READ feedType WRITE setFeedType NOTIFY
                       feedTypeChanged)

public:
    explicit AnyFeedListModel(QObject *parent = nullptr);

    enum AnyFeedListModelFeedType { LikeFeedType = 0, RepostFeedType = 1 };
    Q_ENUM(AnyFeedListModelFeedType)

    Q_INVOKABLE void getLatest();

    QString targetDid() const;
    void setTargetDid(const QString &newTargetDid);
    AnyFeedListModelFeedType feedType() const;
    void setFeedType(AnyFeedListModelFeedType newFeedType);

signals:
    void targetDidChanged();
    void feedTypeChanged();

private:
    QStringList m_cueGetPost; // uri
    QHash<QString, AtProtocolType::ComAtprotoRepoListRecords::Record> m_recordHash;

    QString m_targetDid;
    AnyFeedListModelFeedType m_feedType;

    void getPosts();

    template<typename T>
    QString appendGetPostCue(const QVariant &record);
};

#endif // ANYFEEDLISTMODEL_H
