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
            if (request.url().path().endsWith("/xrpc/com.atproto.server.createSession")) {
                QHttpServerResponder::HeaderList headers {
                    std::make_pair(QByteArray("ratelimit-limit"), QByteArray("30")),
                    std::make_pair(QByteArray("ratelimit-remaining"), QByteArray("10")),
                    std::make_pair(QByteArray("ratelimit-reset"), QByteArray("1694914267")),
                    std::make_pair(QByteArray("ratelimit-policy"), QByteArray("30;w=300"))
                };
                if (request.url().path().endsWith("/limit/xrpc/com.atproto.server.createSession")) {
                    makeResponder(request, socket)
                            .write(json.toUtf8(), headers,
                                   QHttpServerResponder::StatusCode::Unauthorized);
                } else {
                    makeResponder(request, socket)
                            .write(json.toUtf8(), headers, QHttpServerResponder::StatusCode::Ok);
                }
            } else {
                makeResponder(request, socket)
                        .write(json.toUtf8(),
                               m_MimeDb.mimeTypeForFile("result.json").name().toUtf8(),
                               QHttpServerResponder::StatusCode::Ok);
            }
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
