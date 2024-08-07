#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <QtHttpServer>

class WebServer : public QAbstractHttpServer
{
    Q_OBJECT
public:
    explicit WebServer(QObject *parent = nullptr);

    bool handleRequest(const QHttpServerRequest &request, QTcpSocket *socket) override;

    static QString convertResoucePath(const QUrl &url);
    static bool readFile(const QString &path, QByteArray &data);
signals:
    void receivedPost(const QHttpServerRequest &request, bool &result, QString &json);

private:
    QMimeDatabase m_MimeDb;

    bool verifyHttpHeader(const QHttpServerRequest &request) const;
};

#endif // WEBSERVER_H
