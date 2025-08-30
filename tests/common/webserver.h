#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <QtHttpServer>

class WebServer : public QAbstractHttpServer
{
    Q_OBJECT
public:
    explicit WebServer(QObject *parent = nullptr);

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    bool handleRequest(const QHttpServerRequest &request, QTcpSocket *socket) override;
#else
    virtual bool handleRequest(const QHttpServerRequest &request, QHttpServerResponder &responder);
    virtual void missingHandler(const QHttpServerRequest &request, QHttpServerResponder &responder);
    quint16 listen(const QHostAddress &address = QHostAddress::Any, quint16 port = 0);
#endif

    void reset();

    static QString convertResoucePath(const QUrl &url);
    static bool readFile(const QString &path, QByteArray &data);
signals:
    void receivedPost(const QHttpServerRequest &request, bool &result, QString &json,
                      QHttpServerResponder::StatusCode &status_code);

private:
    QMimeDatabase m_MimeDb;
    int m_videoGetJobStatus;
    bool verifyHttpHeader(const QHttpServerRequest &request) const;
};

#endif // WEBSERVER_H
