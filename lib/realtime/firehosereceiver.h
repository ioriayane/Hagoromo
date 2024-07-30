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

    static FirehoseReceiver *getInstance();

    void start();
    void stop();

    void appendSelector(AbstractPostSelector *selector);
    void removeSelector(QObject *parent);
    void removeAllSelector();
    AbstractPostSelector *getSelector(QObject *parent);
    bool containsSelector(QObject *parent);
    int countSelector() const;

#ifdef HAGOROMO_UNIT_TEST
    void testReceived(const QJsonObject &json);
#endif

    QString serviceEndpoint() const;
    void setServiceEndpoint(const QString &newServiceEndpoint);

signals:
    void errorOccured(const QString &code, const QString &message);
    void connectedToService();
    void disconnectFromService();
    void receiving(bool status);

private:
    QHash<QObject *, QPointer<AbstractPostSelector>> m_selectorHash;
    AtProtocolInterface::ComAtprotoSyncSubscribeReposEx m_client;
    QTimer m_wdgTimer;

    QString m_serviceEndpoint;
};

}

#endif // FIREHOSERECEIVER_H
