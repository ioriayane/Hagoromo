#ifndef FIREHOSERECEIVER_H
#define FIREHOSERECEIVER_H

#include "abstractpostselector.h"
#include "extension/com/atproto/sync/comatprotosyncsubscribereposex.h"

#include <QElapsedTimer>
#include <QMutex>
#include <QObject>
#include <QPointer>
#include <QThread>
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
        Connecting,
        HostLookup,
        Bound,
        Closing,
        Error,
    };

    static FirehoseReceiver *getInstance();

    void start();
    void stop();

    void appendSelector(AbstractPostSelector *selector);
    void removeSelector(QObject *parent);
    void removeAllSelector();
    AbstractPostSelector *getSelector(QObject *parent) const;
    bool containsSelector(QObject *parent) const;
    int countSelector() const;
    bool selectorIsReady(QObject *parent);

#ifdef QT_DEBUG // HAGOROMO_UNIT_TEST
    bool forUnittest;
    void testReceived(const QJsonObject &json);
    void testUpdateReceivedCursorState(const QJsonObject &json);
    QString testTakeCursor();
    void testResetCursorState();
    void testUpdateCatchingUpState(const QJsonObject &json);
    void testSetCatchingUp(bool newCatchingUp);
    QStringList testSubscribeCollections() const;
#endif

    QString serviceEndpoint() const;
    void setServiceEndpoint(const QString &newServiceEndpoint);
    void changeServiceEndpoint(const QString &newServiceEndpoint);

    FirehoseReceiverStatus status() const;
    void setStatus(FirehoseReceiverStatus newStatus);

    QHash<QString, QString> nsidsReceivePerSecond() const;

    bool catchingUp() const;

signals:
    void errorOccurred(const QString &code, const QString &message);
    void connectedToService();
    void disconnectFromService();
    void receivingChanged(bool status);
    void statusChanged(FirehoseReceiverStatus newStatus);
    void analysisChanged();
    void judgeSelectionAndReaction(const QJsonObject &object);
    void serviceEndpointChanged(const QString &endpoint);
    void catchingUpChanged(bool catchingUp);

private:
    void analizeReceivingData(const QJsonObject &json, const qsizetype size);
    void appendThreadSelector(AbstractPostSelector *selector);
    void removeThreadSelector(QObject *parent);
    void updateReceivedCursorState(const QJsonObject &json);
    QString getCursor() const;
    QString getInitialCursor() const;
    QString takeCursor();
    void setCatchingUp(bool newCatchingUp);
    void updateCatchingUpState(const QJsonObject &json);
    QStringList subscribeCollections() const;

    QHash<QObject *, QPointer<AbstractPostSelector>> m_selectorHash;
    QHash<QObject *, QPointer<QThread>> m_selectorThreadHash;
    AtProtocolInterface::ComAtprotoSyncSubscribeReposEx m_client;
    QTimer m_wdgTimer;
    int m_wdgCounter;
    QElapsedTimer m_analysisTimer;
    QThread m_clientThread;
    QMutex m_selectorMutex;

    QString m_serviceEndpoint;
    FirehoseReceiverStatus m_status;

    QHash<QString, int> m_nsidsCount; // QHash<nsid, count>
    QHash<QString, QString> m_nsidsReceivePerSecond; // QHash<nsid, receive/sec>
    qsizetype m_receivedDataSize; // byte
    qint64 m_timeOfReceivedData; // 最終受信時刻(ローカル時刻)
    qint64 m_lastSeq; // JetStreamの最終受信seq(カーソル再開用)
    bool m_initialCursorRequested; // 起動後最初の接続でさかのぼり受信を要求したか
    bool m_catchingUp; // さかのぼり受信で現在時刻に追いつくまでの間
    QElapsedTimer m_catchingUpTimer;
    bool m_restartAfterDisconnect; // 切断後に再接続する(追いついたあとの購読の切り替え用)
};

}

#endif // FIREHOSERECEIVER_H
