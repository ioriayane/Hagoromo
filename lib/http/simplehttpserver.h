#ifndef SIMPLEHTTPSERVER_H
#define SIMPLEHTTPSERVER_H

#include <QHttpServer>
#include <QTimer>

class SimpleHttpServer : public QAbstractHttpServer
{
    Q_OBJECT
public:
    explicit SimpleHttpServer(QObject *parent = nullptr);

    void setTimeout(int sec);
    void clearTimeout();
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    bool handleRequest(const QHttpServerRequest &request, QTcpSocket *socket) override;
#else
    virtual bool handleRequest(const QHttpServerRequest &request, QHttpServerResponder &responder);
    virtual void missingHandler(const QHttpServerRequest &request, QHttpServerResponder &responder);
    quint16 listen(const QHostAddress &address = QHostAddress::Any, quint16 port = 0);
#endif
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
