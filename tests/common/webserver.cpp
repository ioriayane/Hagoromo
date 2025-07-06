#include "webserver.h"

WebServer::WebServer(QObject *parent) : QAbstractHttpServer(parent), m_videoGetJobStatus(0) { }

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
bool WebServer::handleRequest(const QHttpServerRequest &request, QTcpSocket *socket)
#    define MAKE_RESPONDER makeResponder(request, socket)
#else
#    define MAKE_RESPONDER responder
bool WebServer::handleRequest(const QHttpServerRequest &request, QHttpServerResponder &responder)
#endif
{
    if (!verifyHttpHeader(request)) {
        MAKE_RESPONDER.write(QHttpServerResponder::StatusCode::BadRequest);
    } else if (request.url().path().contains("//")) {
        MAKE_RESPONDER.write(QHttpServerResponder::StatusCode::NotFound);
    } else if (request.method() == QHttpServerRequest::Method::Post) {
        bool result = false;
        QString json;
        emit receivedPost(request, result, json);
        if (result) {
            if (request.url().path().endsWith("/xrpc/com.atproto.server.createSession")) {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
                QHttpServerResponder::HeaderList headers {
                    std::make_pair(QByteArray("content-type"),
                                   QByteArray("application/json; charset=utf-8")),
                    std::make_pair(QByteArray("ratelimit-limit"), QByteArray("30")),
                    std::make_pair(QByteArray("ratelimit-remaining"), QByteArray("10")),
                    std::make_pair(QByteArray("ratelimit-reset"), QByteArray("1694914267")),
                    std::make_pair(QByteArray("ratelimit-policy"), QByteArray("30;w=300"))
                };
#else
                QHttpHeaders headers;
                headers.append("content-type", "application/json; charset=utf-8");
                headers.append("ratelimit-reset", "1694914267");
                headers.append("ratelimit-limit", "30");
                headers.append("ratelimit-remaining", "10");
                headers.append("ratelimit-policy", "30;w=300");
#endif
                if (request.url().path().endsWith("/limit/xrpc/com.atproto.server.createSession")) {
                    MAKE_RESPONDER.write(json.toUtf8(), headers,
                                         QHttpServerResponder::StatusCode::Unauthorized);
                } else {
                    MAKE_RESPONDER.write(json.toUtf8(), headers,
                                         QHttpServerResponder::StatusCode::Ok);
                }
            } else {
                MAKE_RESPONDER.write(json.toUtf8(),
                                     m_MimeDb.mimeTypeForFile("result.json").name().toUtf8(),
                                     QHttpServerResponder::StatusCode::Ok);
            }
        } else {
            MAKE_RESPONDER.write(QHttpServerResponder::StatusCode::InternalServerError);
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
        } else if (path.endsWith("xrpc/app.bsky.video.getJobStatus") && m_videoGetJobStatus > 0) {
            path += "_" + QString::number(m_videoGetJobStatus);
            m_videoGetJobStatus++;
        }
        if (request.query().hasQueryItem("cursor")) {
            path += "_" + request.query().queryItemValue("cursor", QUrl::DecodeReserved);
        }
        qDebug().noquote() << "SERVER PATH=" << path;
        if (!QFile::exists(path)) {
            MAKE_RESPONDER.write(QHttpServerResponder::StatusCode::NotFound);
        } else {
            QFileInfo file_info(request.url().path());
            QByteArray data;
            QString mime_type = "application/json";
            if (path.endsWith("/xrpc/com.atproto.sync.getRepo")) {
                mime_type = "application/vnd.ipld.car";
            }
            if (WebServer::readFile(path, data)) {
                data.replace("{{SERVER_PORT_NO}}", QString::number(request.localPort()).toUtf8());
                MAKE_RESPONDER.write(data, mime_type.toUtf8(),
                                     QHttpServerResponder::StatusCode::Ok);
            } else {
                MAKE_RESPONDER.write(QHttpServerResponder::StatusCode::InternalServerError);
            }
        }
    }
    return true;
}

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#else
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

void WebServer::reset()
{
    m_videoGetJobStatus = 0;
}
#endif

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
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    if (request.headers().contains("atproto-accept-labelers")) {
        QStringList dids = request.headers().value("atproto-accept-labelers").toString().split(",");
        for (const auto &did : dids) {
            if (!did.startsWith("did:")) {
                return false;
            }
        }
    }
#else
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
#endif
    return true;
}
