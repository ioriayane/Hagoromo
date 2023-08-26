#ifndef ATPABSTRACTLISTMODEL_H
#define ATPABSTRACTLISTMODEL_H

#include "atprotocol/accessatprotocol.h"
#include "atprotocol/lexicons.h"
#include "tools/configurablelabels.h"

#include <QAbstractListModel>
#include <QObject>
#include <QTimer>
#include <functional>

struct PostCueItem
{
    QString cid;
    QString indexed_at;
    QDateTime reference_time;
    AtProtocolType::AppBskyFeedDefs::FeedViewPostReasonType reason_type =
            AtProtocolType::AppBskyFeedDefs::FeedViewPostReasonType::none;
};

class AtpAbstractListModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
    Q_PROPERTY(bool autoLoading READ autoLoading WRITE setAutoLoading NOTIFY autoLoadingChanged)
    Q_PROPERTY(int loadingInterval READ loadingInterval WRITE setLoadingInterval NOTIFY
                       loadingIntervalChanged)
    Q_PROPERTY(int displayInterval READ displayInterval WRITE setDisplayInterval NOTIFY
                       displayIntervalChanged)

    Q_PROPERTY(QString service READ service CONSTANT)
    Q_PROPERTY(QString did READ did CONSTANT)
    Q_PROPERTY(QString handle READ handle CONSTANT)
    Q_PROPERTY(QString email READ email CONSTANT)
    Q_PROPERTY(QString accessJwt READ accessJwt CONSTANT)
    Q_PROPERTY(QString refreshJwt READ refreshJwt CONSTANT)

public:
    explicit AtpAbstractListModel(QObject *parent = nullptr);

    Q_INVOKABLE void clear();

    AtProtocolInterface::AccountData account() const;
    Q_INVOKABLE void setAccount(const QString &service, const QString &did, const QString &handle,
                                const QString &email, const QString &accessJwt,
                                const QString &refreshJwt);
    virtual Q_INVOKABLE int indexOf(const QString &cid) const = 0;
    virtual Q_INVOKABLE QString getRecordText(const QString &cid) = 0;
    Q_INVOKABLE void translate(const QString &cid);
    Q_INVOKABLE void reflectVisibility();

    bool running() const;
    void setRunning(bool newRunning);
    bool autoLoading() const;
    void setAutoLoading(bool newAutoLoading);
    int loadingInterval() const;
    void setLoadingInterval(int newLoadingInterval);
    int displayInterval() const;
    void setDisplayInterval(int newDisplayInterval);
    QString service() const;
    QString did() const;
    QString handle() const;
    QString email() const;
    QString accessJwt() const;
    QString refreshJwt() const;

signals:
    void errorOccured(const QString &message);
    void runningChanged();
    void autoLoadingChanged();
    void loadingIntervalChanged();
    void displayIntervalChanged();

public slots:
    virtual Q_INVOKABLE void getLatest() = 0;

protected:
    QString formatDateTime(const QString &value, const bool is_long = false) const;
    QString copyRecordText(const QVariant &value) const;
    void displayQueuedPosts();
    virtual void finishedDisplayingQueuedPosts() = 0;
    virtual bool checkVisibility(const QString &cid) = 0;
    void updateContentFilterLabels(std::function<void()> callback);
    ConfigurableLabelStatus
    getContentFilterStatus(const QList<AtProtocolType::ComAtprotoLabelDefs::Label> &labels,
                           const bool for_media) const;
    bool getContentFilterMatched(const QList<AtProtocolType::ComAtprotoLabelDefs::Label> &labels,
                                 const bool for_media) const;
    QString getContentFilterMessage(const QList<AtProtocolType::ComAtprotoLabelDefs::Label> &labels,
                                    const bool for_media) const;
    bool getQuoteFilterMatched(const AtProtocolType::AppBskyFeedDefs::PostView &post) const;
    QStringList getLabels(const QList<AtProtocolType::ComAtprotoLabelDefs::Label> &labels) const;
    QStringList getLaunguages(const QVariant &record) const;
    QString getVia(const QVariant &record) const;

    // これで取得したポストの順番を管理して実態はm_viewPostHashで管理
    // checkVisibility(cid)の結果次第で間引かれる
    QList<QString> m_cidList;
    // こっちは取得したcidをすべて表示する予定の順番で記録する
    // displayQueuedPosts()を使ってcidのリストを構成しないと使わない
    QList<QString> m_originalCidList;
    QList<PostCueItem> m_cuePost;

    QHash<QString, QString> m_translations; // QHash<cid, translation>

    ConfigurableLabels m_contentFilterLabels;

private:
    int searchInsertPosition(const QString &cid);

    QTimer m_timer;
    AtProtocolInterface::AccountData m_account;

    bool m_running;
    int m_loadingInterval;
    int m_displayInterval;
};

#endif // ATPABSTRACTLISTMODEL_H
