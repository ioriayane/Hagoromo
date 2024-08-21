#include "simplehttpserver.h"
#include <QTimer>

SimpleHttpServer::SimpleHttpServer(QObject *parent) : QAbstractHttpServer { parent } { }

void SimpleHttpServer::setTimeout(int sec)
{
    QTimer::singleShot(sec * 1000, [=]() { emit timeout(); });
}

bool SimpleHttpServer::handleRequest(const QHttpServerRequest &request, QTcpSocket *socket)
{
    bool result = false;
    QByteArray data;
    QByteArray mime_type;
    emit received(request, result, data, mime_type);
    if (result) {
        makeResponder(request, socket).write(data, mime_type, QHttpServerResponder::StatusCode::Ok);
    } else {
        makeResponder(request, socket).write(QHttpServerResponder::StatusCode::InternalServerError);
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
