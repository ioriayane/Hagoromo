#ifndef REALTIMEFEEDLISTMODEL_H
#define REALTIMEFEEDLISTMODEL_H

#include "timeline/timelinelistmodel.h"
#include "realtime/firehosereceiver.h"

class RealtimeFeedListModel : public TimelineListModel
{
    Q_OBJECT

    Q_PROPERTY(QString selectorJson READ selectorJson WRITE setSelectorJson NOTIFY
                       selectorJsonChanged FINAL)
    Q_PROPERTY(bool receiving READ receiving WRITE setReceiving NOTIFY receivingChanged FINAL)
public:
    explicit RealtimeFeedListModel(QObject *parent = nullptr);
    ~RealtimeFeedListModel();

    Q_INVOKABLE bool getLatest();
    Q_INVOKABLE bool getNext();
    Q_INVOKABLE bool like(int row);

    QString selectorJson() const;
    void setSelectorJson(const QString &newSelectorJson);
    bool receiving() const;
    void setReceiving(bool newReceiving);

signals:
    void selectorJsonChanged();
    void receivingChanged();

private:
    void getFollowing();
    void getFollowers();
    void getListMembers();
    void finishGetting(RealtimeFeed::AbstractPostSelector *selector);
    void copyFollows(const QList<AtProtocolType::AppBskyActorDefs::ProfileView> &follows,
                     bool is_following);
    void copyListMembers(const QString &list_uri,
                         const QList<AtProtocolType::AppBskyGraphDefs::ListItemView> &items);
    void getPostThread();

    bool m_runningCue;
    QList<RealtimeFeed::OperationInfo> m_cueGetPostThread;
    QList<RealtimeFeed::UserInfo> m_followings;
    QList<RealtimeFeed::UserInfo> m_followers;
    QMap<QString, QList<RealtimeFeed::UserInfo>> m_list_members; // QMap<list_uri, List<UserInfo>>
    QStringList m_get_list_cue;

    QString m_cursor;
    QString m_selectorJson;
    bool m_receiving;
};

#endif // REALTIMEFEEDLISTMODEL_H
