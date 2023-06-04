#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <QtHttpServer>

class WebServer : public QAbstractHttpServer
{
public:
    WebServer();

    bool handleRequest(const QHttpServerRequest &request, QTcpSocket *socket) override;

private:
    QMimeDatabase m_MimeDb;
};

#endif // WEBSERVER_H
