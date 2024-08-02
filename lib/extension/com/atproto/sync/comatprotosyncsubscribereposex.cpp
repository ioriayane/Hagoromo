#include "comatprotosyncsubscribereposex.h"

#include "tools/cardecoder.h"

#include <QtNetwork/QSslError>

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCborMap>
#include <QDebug>
#include <QFile>
#include <QMetaEnum>

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

    CarDecoder decoder(true);
    int offset = 0;
    bool is_error_frame = false;
    QString payload_type;
    while (offset < message.length()) {
        offset += decoder.decodeCbor(message.mid(offset), "__message__");

        // qDebug().noquote() << "decodeCbor" << offset;
        QJsonObject json = decoder.json("__message__");
        // qDebug().noquote() << QJsonDocument(json).toJson(QJsonDocument::Compact);

        if (json.contains("op")) {
            // header
            // qDebug().noquote() << QJsonDocument(json).toJson(QJsonDocument::Compact);
            is_error_frame = (json.value("op").toInt() != 1);
            payload_type = json.value("t").toString();
        } else if (is_error_frame && json.contains("error")) {
            // error payload
            qDebug().noquote() << QJsonDocument(json).toJson();
            emit errorOccured(json.value("error").toString(), json.value("message").toString());
            m_webSocket.close();
        } else if (json.contains("seq")) {
            if (!m_payloadTypeList.contains(payload_type)) {
                // unknown payload type
                // skip
                qDebug().noquote() << "Unknown payload type" << payload_type;
                qDebug().noquote() << QJsonDocument(json).toJson();
            } else {
                // payload
                emit received(payload_type, json);
            }
        } else {
            // decode error
            qDebug().noquote() << QJsonDocument(json).toJson();
            emit errorOccured("DecodeError", "Unknown data format.");
            m_webSocket.close();
        }
    }

    if (offset != message.length()) {
        qDebug().noquote() << "Invalid offset ?";
        emit errorOccured("InvalidDataSize",
                          "The size of the decoded data does not match the total.");
        m_webSocket.close();
    }
}
}
