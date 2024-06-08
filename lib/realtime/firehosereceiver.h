#ifndef FIREHOSERECEIVER_H
#define FIREHOSERECEIVER_H

#include "abstractpostselector.h"
#include "extension/com/atproto/sync/comatprotosyncsubscribereposex.h"

#include <QObject>

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

    QString serviceEndpoint() const;
    void setServiceEndpoint(const QString &newServiceEndpoint);

signals:
    void errorOccured(const QString &code, const QString &message);
    void connectedToService();
    void disconnectFromService();

private:
    QHash<QObject *, AbstractPostSelector *> m_selectorHash;
    ComAtprotoSyncSubscribeReposEx m_client;

    QString m_serviceEndpoint;
};

}

#endif // FIREHOSERECEIVER_H
