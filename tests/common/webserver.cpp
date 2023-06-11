#include "webserver.h"

WebServer::WebServer(QObject *parent) : QAbstractHttpServer(parent) { }

bool WebServer::handleRequest(const QHttpServerRequest &request, QTcpSocket *socket)
{
    QFileInfo file_info(request.url().path());
    QString path = ":" + file_info.filePath();

    if (request.method() == QHttpServerRequest::Method::Post) {
        bool result = false;
        QString json;
        emit receivedPost(request, result, json);
        if (result) {
            makeResponder(request, socket)
                    .write(json.toUtf8(), m_MimeDb.mimeTypeForFile("result.json").name().toUtf8(),
                           QHttpServerResponder::StatusCode::Ok);
        } else {
            makeResponder(request, socket)
                    .write(QHttpServerResponder::StatusCode::InternalServerError);
        }
    } else if (!QFile::exists(path)) {
        makeResponder(request, socket).write(QHttpServerResponder::StatusCode::NotFound);
    } else {
        QFile file(path);
        if (file.open(QFile::ReadOnly)) {
            makeResponder(request, socket)
                    .write(file.readAll(), m_MimeDb.mimeTypeForFile(file_info).name().toUtf8(),
                           QHttpServerResponder::StatusCode::Ok);
            file.close();
        } else {
            makeResponder(request, socket)
                    .write(QHttpServerResponder::StatusCode::InternalServerError);
        }
    }
    return true;
}
