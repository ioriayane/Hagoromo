#include "simplehttpserver.h"

SimpleHttpServer::SimpleHttpServer(QObject *parent) : QAbstractHttpServer { parent }
{
    connect(&m_timeout, &QTimer::timeout, this, [=]() { emit timeout(); });
}

void SimpleHttpServer::setTimeout(int sec)
{
    if (m_timeout.isActive()) {
        qDebug().noquote() << "SimpleHttpServer::Restart timeout:" << sec;
        m_timeout.stop();
    } else {
        qDebug().noquote() << "SimpleHttpServer::Start timeout:" << sec;
    }
    m_timeout.start(sec * 1000);
}

void SimpleHttpServer::clearTimeout()
{
    if (m_timeout.isActive()) {
        qDebug().noquote() << "SimpleHttpServer::Stop timeout";
        m_timeout.stop();
    }
}

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#else
void SimpleHttpServer::missingHandler(const QHttpServerRequest &request,
                                      QHttpServerResponder &&responder)
{
    Q_UNUSED(request)
    Q_UNUSED(responder)
}
#endif

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
bool SimpleHttpServer::handleRequest(const QHttpServerRequest &request, QTcpSocket *socket)
#    define MAKE_RESPONDER makeResponder(request, socket)
#else
#    define MAKE_RESPONDER responder
bool SimpleHttpServer::handleRequest(const QHttpServerRequest &request,
                                     QHttpServerResponder &responder)
#endif
{
    bool result = false;
    QByteArray data;
    QByteArray mime_type;
    emit received(request, result, data, mime_type);
    if (result) {
        MAKE_RESPONDER.write(data, mime_type, QHttpServerResponder::StatusCode::Ok);
    } else {
        MAKE_RESPONDER.write(QHttpServerResponder::StatusCode::InternalServerError);
    }
    return true;
}

QString SimpleHttpServer::convertResoucePath(const QUrl &url)
{
    QFileInfo file_info(url.path());
    return ":" + file_info.filePath();
}

bool SimpleHttpServer::readFile(const QString &path, QByteArray &data)
{
    QFile file(path);
    if (file.open(QFile::ReadOnly)) {
        data = file.readAll();
        file.close();
        return true;
    } else {
        return false;
    }
}
