#ifndef ATPABSTRACTLISTMODEL_H
#define ATPABSTRACTLISTMODEL_H

#include "atprotocol/accessatprotocol.h"
#include "atprotocol/lexicons.h"

#include <QAbstractListModel>
#include <QObject>
#include <QTimer>

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

    AtProtocolInterface::AccountData account() const;
    Q_INVOKABLE void setAccount(const QString &service, const QString &did, const QString &handle,
                                const QString &email, const QString &accessJwt,
                                const QString &refreshJwt);
    virtual Q_INVOKABLE int indexOf(const QString &cid) const = 0;
    virtual Q_INVOKABLE QString getRecordText(const QString &cid) = 0;
    Q_INVOKABLE void translate(const QString &cid);

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
    void runningChanged();
    void autoLoadingChanged();
    void loadingIntervalChanged();
    void displayIntervalChanged();

public slots:
    virtual Q_INVOKABLE void getLatest() = 0;

protected:
    QString formatDateTime(const QString &value) const;
    QString copyRecordText(const QVariant &value) const;
    void displayQueuedPosts();
    virtual void finishedDisplayingQueuedPosts() = 0;

    QList<QString> m_cidList; // これで取得したポストの順番を管理して実態はm_viewPostHashで管理
    QList<PostCueItem> m_cuePost;
    int m_displayInterval;

    QHash<QString, QString> m_translations; // QHash<cid, translation>

private:
    QTimer m_timer;
    AtProtocolInterface::AccountData m_account;

    QHash<QString, QString> m_recordTextCache;

    bool m_running;
    int m_loadingInterval;
};

#endif // ATPABSTRACTLISTMODEL_H
