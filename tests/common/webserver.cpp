#include "webserver.h"

WebServer::WebServer() { }

bool WebServer::handleRequest(const QHttpServerRequest &request, QTcpSocket *socket)
{
    QFileInfo file_info(request.url().path());
    QString path = ":" + file_info.filePath();

    if (!QFile::exists(path)) {
        makeResponder(request, socket).write(QHttpServerResponder::StatusCode::NotFound);
    } else {
        QFile *file = new QFile(path);
        if (file->open(QFile::ReadOnly)) {
            makeResponder(request, socket)
                    .write(file, m_MimeDb.mimeTypeForFile(file_info).name().toUtf8(),
                           QHttpServerResponder::StatusCode::Ok);
        } else {
            makeResponder(request, socket)
                    .write(QHttpServerResponder::StatusCode::InternalServerError);
        }
    }
    return true;
}
