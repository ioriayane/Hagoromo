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
    explicit ComAtprotoSyncSubscribeReposEx(QObject *parent = nullptr);

    void open(const QUrl &url);
    void close();
    QAbstractSocket::SocketState state() const;

signals:
    void errorOccured(const QString &code, const QString &message);
    void received(const QString &type, const QJsonObject &json);
    void connectedToService();
    void disconnectFromService();

public slots:
    void onConnected();
    void onDisconnected();
    void onBinaryMessageReceived(const QByteArray &message);

private:
    QWebSocket m_webSocket;
    QStringList m_payloadTypeList;
    QStringList m_operationTypeList;
};

}

#endif // COMATPROTOSYNCSUBSCRIBEREPOSEX_H
