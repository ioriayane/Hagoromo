#ifndef COMATPROTOSYNCSUBSCRIBEREPOSEX_H
#define COMATPROTOSYNCSUBSCRIBEREPOSEX_H

#include <QJsonObject>
#include <QObject>

#include <QtWebSockets/QWebSocket>

namespace AtProtocolInterface {

class ComAtprotoSyncSubscribeReposEx : public QObject
{
    Q_OBJECT
public:
    enum SubScribeMode : int {
        Firehose,
        JetStream,
    };

    explicit ComAtprotoSyncSubscribeReposEx(QObject *parent = nullptr);

    void open(const QUrl &url, SubScribeMode mode = SubScribeMode::Firehose);
    void close();
    QAbstractSocket::SocketState state() const;

    static QJsonObject convertJetStreamMessage(const QByteArray &message);

#ifdef QT_DEBUG // HAGOROMO_UNIT_TEST
    void testMessageReceivedFromJetStream(const QByteArray &message);
#endif

signals:
    void errorOccurred(const QString &code, const QString &message);
    // message : JetStreamの受信データ(Firehoseは複数のデータをまとめて受信するので空)
    void received(const QString &type, const QJsonObject &json, const qsizetype size,
                  const QByteArray &message);
    void connectedToService();
    void disconnectFromService();
    void socketStateChanged(QAbstractSocket::SocketState state);

public slots:
    void onConnected();
    void onDisconnected();
    void onBinaryMessageReceived(const QByteArray &message);
    void onTextMessageReceived(const QString &message);

private:
    void messageReceivedFromFirehose(const QByteArray &message);
    void messageReceivedFromJetStream(const QByteArray &message);
    static QJsonObject convertJetStreamCommit(const QJsonObject &json_src);
    void closeWebSocket();

    QWebSocket m_webSocket;
    QStringList m_payloadTypeList;
    SubScribeMode m_subscribeMode;
};

}

#endif // COMATPROTOSYNCSUBSCRIBEREPOSEX_H
