#ifndef REALTIMEFEEDLISTMODEL_H
#define REALTIMEFEEDLISTMODEL_H

#include "timelinelistmodel.h"
#include "realtime/firehosereceiver.h"

class RealtimeFeedListModel : public TimelineListModel
{
    Q_OBJECT

    Q_PROPERTY(QString selectorJson READ selectorJson WRITE setSelectorJson NOTIFY
                       selectorJsonChanged FINAL)

public:
    explicit RealtimeFeedListModel(QObject *parent = nullptr);
    ~RealtimeFeedListModel();

    Q_INVOKABLE bool getLatest();
    Q_INVOKABLE bool getNext();

    QString selectorJson() const;
    void setSelectorJson(const QString &newSelectorJson);
signals:
    void selectorJsonChanged();

private:
    void getFollowing();
    void getFollowers();
    void finishGetting(RealtimeFeed::AbstractPostSelector *selector);
    void copyFollows(const QList<AtProtocolType::AppBskyActorDefs::ProfileView> &follows,
                     bool is_following);

    QList<RealtimeFeed::UserInfo> m_followings;
    QList<RealtimeFeed::UserInfo> m_followers;

    QString m_cursor;
    QString m_selectorJson;
};

#endif // REALTIMEFEEDLISTMODEL_H
