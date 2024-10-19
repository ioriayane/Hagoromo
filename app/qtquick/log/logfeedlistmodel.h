#ifndef LOGFEEDLISTMODEL_H
#define LOGFEEDLISTMODEL_H

#include "../timeline/timelinelistmodel.h"

class LogFeedListModel : public TimelineListModel
{
    Q_OBJECT

    Q_PROPERTY(QString selectCondition READ selectCondition WRITE setSelectCondition NOTIFY
                       selectConditionChanged)
    Q_PROPERTY(LogFeedListModelFeedType feedType READ feedType WRITE setFeedType NOTIFY
                       feedTypeChanged FINAL)

    Q_PROPERTY(QString targetDid READ targetDid WRITE setTargetDid NOTIFY targetDidChanged)
    Q_PROPERTY(
            QString targetHandle READ targetHandle WRITE setTargetHandle NOTIFY targetHandleChanged)
    Q_PROPERTY(
            QString targetAvatar READ targetAvatar WRITE setTargetAvatar NOTIFY targetAvatarChanged)
public:
    explicit LogFeedListModel(QObject *parent = nullptr);

    enum LogFeedListModelFeedType { DailyFeedType = 0, MonthlyFeedType = 1, WordsFeedType = 2 };
    Q_ENUM(LogFeedListModelFeedType)

    Q_INVOKABLE bool getLatest();
    Q_INVOKABLE bool getNext();

    QString selectCondition() const;
    void setSelectCondition(const QString &newSelectCondition);
    LogFeedListModel::LogFeedListModelFeedType feedType() const;
    void setFeedType(LogFeedListModelFeedType newFeedType);
    QString targetDid() const;
    void setTargetDid(const QString &newTargetDid);

    QString targetHandle() const;
    void setTargetHandle(const QString &newTargetHandle);

    QString targetAvatar() const;
    void setTargetAvatar(const QString &newTargetAvatar);

signals:
    void finished(bool success);
    void selectConditionChanged();
    void feedTypeChanged();
    void targetDidChanged();

    void targetHandleChanged();

    void targetAvatarChanged();

protected:
    virtual void finishedDisplayingQueuedPosts();

private:
    QString m_selectCondition;
    LogFeedListModelFeedType m_feedType;
    QString m_targetDid;
    QString m_targetHandle;
    QString m_targetAvatar;
};

#endif // LOGFEEDLISTMODEL_H
