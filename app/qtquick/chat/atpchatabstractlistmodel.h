#ifndef ATPCHATABSTRACTLISTMODEL_H
#define ATPCHATABSTRACTLISTMODEL_H

#include "atprotocol/accessatprotocol.h"
#include "tools/configurablelabels.h"

#include <QAbstractListModel>
#include <QTimer>

class AtpChatAbstractListModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
    Q_PROPERTY(bool autoLoading READ autoLoading WRITE setAutoLoading NOTIFY autoLoadingChanged)
    Q_PROPERTY(int loadingInterval READ loadingInterval WRITE setLoadingInterval NOTIFY
                       loadingIntervalChanged)

public:
    explicit AtpChatAbstractListModel(QObject *parent = nullptr);

    Q_INVOKABLE void clear();

    AtProtocolInterface::AccountData account() const;
    Q_INVOKABLE void setAccount(const QString &service, const QString &did, const QString &handle,
                                const QString &email, const QString &accessJwt,
                                const QString &refreshJwt);
    void setServiceEndpoint(const QString &service_endpoint);

    virtual Q_INVOKABLE bool getLatest() = 0;
    virtual Q_INVOKABLE bool getNext() = 0;
    Q_INVOKABLE void updateRead(const QString &convoId, const QString &messageId);

    bool running() const;
    void setRunning(bool newRunning);
    bool autoLoading() const;
    void setAutoLoading(bool newAutoLoading);
    int loadingInterval() const;
    void setLoadingInterval(int newLoadingInterval);

signals:
    void errorOccured(const QString &code, const QString &message);
    void finishUpdateRead(bool success);
    void runningChanged();
    void autoLoadingChanged();
    void loadingIntervalChanged();

protected:
    void getServiceEndpoint(std::function<void()> callback);
    void checkScopeError(const QString &code, const QString &message);

    void updateContentFilterLabels(std::function<void()> callback);
    QStringList labelerDids() const;
    ConfigurableLabelStatus
    getContentFilterStatus(const QList<AtProtocolType::ComAtprotoLabelDefs::Label> &labels,
                           const bool for_media) const;

    QStringList m_idList;
    QString m_cursor;
    QHash<QString, bool> m_itemRunningHash;

private:
    QTimer m_timer;
    AtProtocolInterface::AccountData m_account;
    bool m_running;
    int m_loadingInterval;
};

#endif // ATPCHATABSTRACTLISTMODEL_H
