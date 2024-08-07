#include "webserver.h"

WebServer::WebServer(QObject *parent) : QAbstractHttpServer(parent) { }

bool WebServer::handleRequest(const QHttpServerRequest &request, QTcpSocket *socket)
{
    if (!verifyHttpHeader(request)) {
        makeResponder(request, socket).write(QHttpServerResponder::StatusCode::BadRequest);
    } else if (request.url().path().contains("//")) {
        makeResponder(request, socket).write(QHttpServerResponder::StatusCode::NotFound);
    } else if (request.method() == QHttpServerRequest::Method::Post) {
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
    } else {
        QString path = WebServer::convertResoucePath(request.url());
        if (path.endsWith("xrpc/app.bsky.graph.getList")) {
            if (request.query().hasQueryItem("list")) {
                path += "_"
                        + request.query()
                                  .queryItemValue("list", QUrl::FullyDecoded)
                                  .split("/")
                                  .last();
            }
        }
        if (request.query().hasQueryItem("cursor")) {
            path += "_" + request.query().queryItemValue("cursor", QUrl::DecodeReserved);
        }
        qDebug().noquote() << "SERVER PATH=" << path;
        if (!QFile::exists(path)) {
            makeResponder(request, socket).write(QHttpServerResponder::StatusCode::NotFound);
        } else {
            QFileInfo file_info(request.url().path());
            QByteArray data;
            QString mime_type = "application/json";
            if (path.endsWith("/xrpc/com.atproto.sync.getRepo")) {
                mime_type = "application/vnd.ipld.car";
            }
            if (WebServer::readFile(path, data)) {
                makeResponder(request, socket)
                        .write(data, mime_type.toUtf8(), QHttpServerResponder::StatusCode::Ok);
            } else {
                makeResponder(request, socket)
                        .write(QHttpServerResponder::StatusCode::InternalServerError);
            }
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

bool WebServer::verifyHttpHeader(const QHttpServerRequest &request) const
{
    if (request.headers().contains("atproto-accept-labelers")) {
        QStringList dids = request.headers().value("atproto-accept-labelers").toString().split(",");
        for (const auto &did : dids) {
            if (!did.startsWith("did:")) {
                return false;
            }
        }
    }
    return true;
}
