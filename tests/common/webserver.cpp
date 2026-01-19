#include "webserver.h"

#include <QTcpServer>

WebServer::WebServer(QObject *parent) : QAbstractHttpServer(parent) { }

bool WebServer::handleRequest(const QHttpServerRequest &request, QHttpServerResponder &responder)
{
    if (!verifyHttpHeader(request)) {
        responder.write(QHttpServerResponder::StatusCode::BadRequest);
    } else if (request.url().path().contains("//")) {
        responder.write(QHttpServerResponder::StatusCode::NotFound);
    } else if (request.method() == QHttpServerRequest::Method::Post) {
        bool result = false;
        QString json;
        emit receivedPost(request, result, json);
        if (result) {
            if (request.url().path().endsWith("/xrpc/com.atproto.server.createSession")) {
                QHttpHeaders headers;
                headers.append("content-type", "application/json; charset=utf-8");
                headers.append("ratelimit-reset", "1694914267");
                headers.append("ratelimit-limit", "30");
                headers.append("ratelimit-remaining", "10");
                headers.append("ratelimit-policy", "30;w=300");
                if (request.url().path().endsWith("/limit/xrpc/com.atproto.server.createSession")) {
                    responder.write(json.toUtf8(), headers,
                                    QHttpServerResponder::StatusCode::Unauthorized);
                } else {
                    responder.write(json.toUtf8(), headers, QHttpServerResponder::StatusCode::Ok);
                }
            } else {
                responder.write(json.toUtf8(),
                                m_MimeDb.mimeTypeForFile("result.json").name().toUtf8(),
                                QHttpServerResponder::StatusCode::Ok);
            }
        } else {
            responder.write(QHttpServerResponder::StatusCode::InternalServerError);
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
            responder.write(QHttpServerResponder::StatusCode::NotFound);
        } else {
            QFileInfo file_info(request.url().path());
            QByteArray data;
            QString mime_type = "application/json";
            if (path.endsWith("/xrpc/com.atproto.sync.getRepo")) {
                mime_type = "application/vnd.ipld.car";
            }
            if (WebServer::readFile(path, data)) {
                data.replace("{{SERVER_PORT_NO}}", QString::number(request.localPort()).toUtf8());
                responder.write(data, mime_type.toUtf8(), QHttpServerResponder::StatusCode::Ok);
            } else {
                responder.write(QHttpServerResponder::StatusCode::InternalServerError);
            }
        }
    }
    return true;
}

void WebServer::missingHandler(const QHttpServerRequest &request, QHttpServerResponder &responder)
{
    Q_UNUSED(request)
    Q_UNUSED(responder)
}

quint16 WebServer::listen(const QHostAddress &address, quint16 port)
{
    auto tcpserver = new QTcpServer(this);
    if (!tcpserver->listen(address, port) || !bind(tcpserver)) {
        tcpserver->deleteLater();
        return 0;
    }
    return tcpserver->serverPort();
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
    QStringList exclude_endopoint = QStringList()
            << QStringLiteral("app.bsky.video.getUploadLimits")
            << QStringLiteral("app.bsky.video.uploadVideo")
            << QStringLiteral("app.bsky.video.getJobStatus");
    for (const auto &header : request.headers().toListOfPairs()) {
        if (header.first.contains("atproto-accept-labelers")) {
            QList<QByteArray> dids = header.second.split(',');
            for (const auto &did : dids) {
                if (!did.startsWith("did:")) {
                    return false;
                }
            }
        } else if (header.first.contains("atproto-proxy")) {
            QString endpoint = request.url().fileName();
            if (endpoint.startsWith("app.bsky.") && !exclude_endopoint.contains(endpoint)
                && header.second == "did:web:api.bsky.app#bsky_appview") {

                // ok
            } else if (endpoint.startsWith("chat.bsky.")
                       && header.second == "did:web:api.bsky.chat#bsky_chat") {
                // ok
            } else {
                qDebug() << "Proxy Error:" << request.url().host() << endpoint << header.first
                         << header.second;
                return false;
            }
        }
    }
    return true;
}
