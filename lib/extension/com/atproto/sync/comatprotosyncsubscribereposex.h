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

signals:
    void errorOccured(const QString &code, const QString &message);
    void received(const QString &type, const QJsonObject &json, const qsizetype size);
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

    QWebSocket m_webSocket;
    QStringList m_payloadTypeList;
    SubScribeMode m_subscribeMode;
};

}

#endif // COMATPROTOSYNCSUBSCRIBEREPOSEX_H
