#ifndef FIREHOSERECEIVER_H
#define FIREHOSERECEIVER_H

#include "abstractpostselector.h"
#include "extension/com/atproto/sync/comatprotosyncsubscribereposex.h"

#include <QObject>
#include <QPointer>
#include <QTimer>

namespace RealtimeFeed {

class FirehoseReceiver : public QObject
{
    Q_OBJECT

private:
    explicit FirehoseReceiver(QObject *parent = nullptr);
    ~FirehoseReceiver();

public:
    FirehoseReceiver(const FirehoseReceiver &) = delete;
    FirehoseReceiver &operator=(const FirehoseReceiver &) = delete;
    FirehoseReceiver(FirehoseReceiver &&) = delete;
    FirehoseReceiver &operator=(FirehoseReceiver &&) = delete;

    enum FirehoseReceiverStatus {
        Disconnected,
        Connected,
        Error,
    };

    static FirehoseReceiver *getInstance();

    void start();
    void stop();

    void appendSelector(AbstractPostSelector *selector);
    void removeSelector(QObject *parent);
    void removeAllSelector();
    AbstractPostSelector *getSelector(QObject *parent);
    bool containsSelector(QObject *parent);
    int countSelector() const;
    bool selectorIsReady(QObject *parent);

#ifdef QT_DEBUG // HAGOROMO_UNIT_TEST
    bool forUnittest;
    void testReceived(const QJsonObject &json);
#endif

    QString serviceEndpoint() const;
    void setServiceEndpoint(const QString &newServiceEndpoint);

    FirehoseReceiverStatus status() const;
    void setStatus(FirehoseReceiverStatus newStatus);

signals:
    void errorOccured(const QString &code, const QString &message);
    void connectedToService();
    void disconnectFromService();
    void receivingChanged(bool status);
    void statusChanged(FirehoseReceiverStatus newStatus);

private:
    QHash<QObject *, QPointer<AbstractPostSelector>> m_selectorHash;
    AtProtocolInterface::ComAtprotoSyncSubscribeReposEx m_client;
    QTimer m_wdgTimer;

    QString m_serviceEndpoint;
    FirehoseReceiverStatus m_status;
};

}

#endif // FIREHOSERECEIVER_H
