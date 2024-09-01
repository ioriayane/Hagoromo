#include "comatprotosyncsubscribereposex.h"

#include "tools/cardecoder.h"

#include <QtNetwork/QSslError>

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCborMap>
#include <QCborArray>
#include <QDebug>
#include <QFile>
#include <QMetaEnum>
#include <QElapsedTimer>

namespace AtProtocolInterface {

ComAtprotoSyncSubscribeReposEx::ComAtprotoSyncSubscribeReposEx(QObject *parent) : QObject { parent }
{
    qDebug().noquote() << "ComAtprotoSyncSubscribeReposEx";

    m_payloadTypeList << "#commit"
                      << "#identity"
                      << "#handle"
                      << "#migrate"
                      << "#tombstone"
                      << "#info"
                      << "#account";
    m_operationTypeList << "app.bsky.feed.post"
                        << "app.bsky.graph.follow"
                        << "app.bsky.feed.repost";

    connect(&m_webSocket, &QWebSocket::connected, this,
            &ComAtprotoSyncSubscribeReposEx::onConnected);
    connect(&m_webSocket, &QWebSocket::binaryMessageReceived, this,
            &ComAtprotoSyncSubscribeReposEx::onBinaryMessageReceived);
    connect(&m_webSocket, &QWebSocket::disconnected, this,
            &ComAtprotoSyncSubscribeReposEx::onDisconnected);

    connect(&m_webSocket, QOverload<const QList<QSslError> &>::of(&QWebSocket::sslErrors), this,
            [this](const QList<QSslError> &errors) {
                // qDebug().noquote() << "onSslErrors";
                // qDebug().noquote() << errors;
                QStringList messages;
                for (const auto &error : errors) {
                    messages.append(error.errorString());
                }
                emit errorOccured("SslError", messages.join("\n"));
            });
    connect(&m_webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this,
            [this](QAbstractSocket::SocketError error) {
                // qDebug().noquote() << "error" << error;
                const QMetaObject &mo = QAbstractSocket::staticMetaObject;
                QMetaEnum metaEnum = mo.enumerator(mo.indexOfEnumerator("SocketError"));
                emit errorOccured("SocketError", QString(metaEnum.valueToKey(error)));
            });
    connect(&m_webSocket, &QWebSocket::stateChanged, this, [](QAbstractSocket::SocketState state) {
        qDebug().noquote() << "StateChanged:" << state;
    });
}

void ComAtprotoSyncSubscribeReposEx::open(const QUrl &url)
{
    if (m_webSocket.state() == QAbstractSocket::UnconnectedState) {
        m_webSocket.open(url);
    }
}

void ComAtprotoSyncSubscribeReposEx::close()
{
    m_webSocket.close();
}

QAbstractSocket::SocketState ComAtprotoSyncSubscribeReposEx::state() const
{
    return m_webSocket.state();
}

void ComAtprotoSyncSubscribeReposEx::onConnected()
{
    qDebug().noquote() << "WebSocket connected";
    emit connectedToService();
}

void ComAtprotoSyncSubscribeReposEx::onDisconnected()
{
    qDebug().noquote() << "WebSocket disconnected";
    emit disconnectFromService();
}

void ComAtprotoSyncSubscribeReposEx::onBinaryMessageReceived(const QByteArray &message)
{
    // qDebug().noquote() << "onBinaryMessageReceived" << message.length();
    QElapsedTimer timer;
    timer.start();

    CarDecoder decoder(true);
    int offset = 0;
    bool is_error_frame = false;
    QString payload_type;
    while (offset < message.length()) {
        QCborValue value = QCborValue::fromCbor(message.mid(offset));

        QCborMap map = value.toMap();
        if (map.isEmpty()) {
            break;
        } else if (map.contains(QStringLiteral("op"))) {
            // header
            is_error_frame = (map.value("op").toInteger(0) != 1);
            payload_type = map.value("t").toString();
        } else if (map.contains(QStringLiteral("error"))) {
            // error payload
            qDebug().noquote() << QJsonDocument(map.toJsonObject()).toJson();
            emit errorOccured(map.value("error").toString(), map.value("message").toString());
            m_webSocket.close();
        } else if (map.contains(QStringLiteral("seq"))) {
            if (!m_payloadTypeList.contains(payload_type)) {
                // unknown payload type
                // skip
                qDebug().noquote() << "Unknown payload type" << payload_type;
                qDebug().noquote() << QJsonDocument(map.toJsonObject()).toJson();
            } else if (payload_type == "#commit") {
                // payload
                for (const auto op : map.value("ops").toArray()) {
                    QStringList items = op.toMap().value("path").toString().split("/");
                    if (m_operationTypeList.contains(items.first())) {
                        decoder.decodeCbor(message.mid(offset), "__message__");
                        QJsonObject json = decoder.json("__message__");
                        emit received(payload_type, json);
                        break;
                    }
                }
            }
        } else {
            // decode error
            qDebug().noquote() << QJsonDocument(map.toJsonObject()).toJson();
            emit errorOccured("DecodeError", "Unknown data format.");
            m_webSocket.close();
        }
        offset += value.toCbor().length();
    }

    if (offset != message.length()) {
        qDebug().noquote() << "Invalid offset ?";
        emit errorOccured("InvalidDataSize",
                          "The size of the decoded data does not match the total.");
        m_webSocket.close();
    }

    // qDebug().noquote() << timer.elapsed();
}
}
