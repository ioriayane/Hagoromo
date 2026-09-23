#ifndef REALTIMEFEEDLISTMODEL_H
#define REALTIMEFEEDLISTMODEL_H

#include "timeline/timelinelistmodel.h"
#include "realtime/firehosereceiver.h"

#include <QSet>
#include <QTimer>

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
    Q_INVOKABLE bool repost(int row);
    Q_INVOKABLE bool like(int row);
    Q_INVOKABLE bool bookmark(int row);

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
    void abortGetting();
    void copyFollows(const QList<AtProtocolType::AppBskyActorDefs::ProfileView> &follows,
                     bool is_following);
    void copyListMembers(const QString &list_uri,
                         const QList<AtProtocolType::AppBskyGraphDefs::ListItemView> &items);
    void getPostThread();
    void updateReactionCount(const QString &cid, TimelineListModel::TimelineListModelRoles role,
                             bool increment);
    void flushReactionCounts();

    bool m_runningCue;
    QList<RealtimeFeed::OperationInfo> m_cueGetPostThread;
    QList<RealtimeFeed::UserInfo> m_followings;
    QList<RealtimeFeed::UserInfo> m_followers;
    QMap<QString, QList<RealtimeFeed::UserInfo>> m_list_members; // QMap<list_uri, List<UserInfo>>
    QStringList m_get_list_cue;

    QString m_cursor;
    QString m_selectorJson;
    bool m_receiving;

    // like/repostカウントの更新頻度が高い場合にdataChangedの発行回数を間引くための仕組み
    // (件数はイベント受信時に即時反映し、GUIへの通知のみタイマーでまとめて行う)
    QTimer m_reactionFlushTimer;
    QHash<QString, QSet<int>> m_dirtyReactionCountRoles; // QHash<cid, roles>
};

#endif // REALTIMEFEEDLISTMODEL_H
