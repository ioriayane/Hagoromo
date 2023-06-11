#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <QtHttpServer>

class WebServer : public QAbstractHttpServer
{
    Q_OBJECT
public:
    explicit WebServer(QObject *parent = nullptr);

    bool handleRequest(const QHttpServerRequest &request, QTcpSocket *socket) override;

signals:
    void receivedPost(const QHttpServerRequest &request, bool &result, QString &json);

private:
    QMimeDatabase m_MimeDb;
};

#endif // WEBSERVER_H
