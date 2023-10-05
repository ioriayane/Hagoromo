#include "webserver.h"

WebServer::WebServer(QObject *parent) : QAbstractHttpServer(parent) { }

bool WebServer::handleRequest(const QHttpServerRequest &request, QTcpSocket *socket)
{
    if (request.method() == QHttpServerRequest::Method::Post) {
        bool result = false;
        QString json;
        emit receivedPost(request, result, json);
        if (result) {
            if (request.url().path().endsWith("/xrpc/com.atproto.server.createSession")) {
                QHttpServerResponder::HeaderList headers {
                    std::make_pair(QByteArray("content-type"),
                                   QByteArray("application/json; charset=utf-8")),
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
    } else if (!QFile::exists(WebServer::convertResoucePath(request.url()))) {
        makeResponder(request, socket).write(QHttpServerResponder::StatusCode::NotFound);
    } else {
        QFileInfo file_info(request.url().path());
        QByteArray data;
        if (WebServer::readFile(WebServer::convertResoucePath(request.url()), data)) {
            makeResponder(request, socket)
                    .write(data, m_MimeDb.mimeTypeForFile(file_info).name().toUtf8(),
                           QHttpServerResponder::StatusCode::Ok);
        } else {
            makeResponder(request, socket)
                    .write(QHttpServerResponder::StatusCode::InternalServerError);
        }
    }
    return true;
}

QString WebServer::convertResoucePath(const QUrl &url)
{
    QFileInfo file_info(url.path());
    return ":" + file_info.filePath();
}

bool WebServer::readFile(const QString &path, QByteArray &data)
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
