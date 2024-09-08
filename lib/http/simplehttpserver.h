#ifndef SIMPLEHTTPSERVER_H
#define SIMPLEHTTPSERVER_H

#include <QtHttpServer>
#include <QTimer>

class SimpleHttpServer : public QAbstractHttpServer
{
    Q_OBJECT
public:
    explicit SimpleHttpServer(QObject *parent = nullptr);

    void setTimeout(int sec);
    void clearTimeout();
    bool handleRequest(const QHttpServerRequest &request, QTcpSocket *socket) override;

    static QString convertResoucePath(const QUrl &url);
    static bool readFile(const QString &path, QByteArray &data);
signals:
    void received(const QHttpServerRequest &request, bool &result, QByteArray &data,
                  QByteArray &mime_type);
    void timeout();

private:
    QTimer m_timeout;
};

#endif // SIMPLEHTTPSERVER_H
