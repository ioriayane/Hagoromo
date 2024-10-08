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

ComAtprotoSyncSubscribeReposEx::ComAtprotoSyncSubscribeReposEx(QObject *parent)
    : QObject { parent }, m_subscribeMode { SubScribeMode::Firehose }
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
    connect(&m_webSocket, &QWebSocket::textMessageReceived, this,
            &ComAtprotoSyncSubscribeReposEx::onTextMessageReceived);
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

void ComAtprotoSyncSubscribeReposEx::open(const QUrl &url, SubScribeMode mode)
{
    if (m_webSocket.state() == QAbstractSocket::UnconnectedState) {
        m_subscribeMode = mode;
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
    if (m_subscribeMode == SubScribeMode::Firehose) {
        messageReceivedFromFirehose(message);
    }
}

void ComAtprotoSyncSubscribeReposEx::onTextMessageReceived(const QString &message)
{
    if (m_subscribeMode == SubScribeMode::JetStream) {
        messageReceivedFromJetStream(message.toUtf8());
    }
}

void ComAtprotoSyncSubscribeReposEx::messageReceivedFromFirehose(const QByteArray &message)
{

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

void ComAtprotoSyncSubscribeReposEx::messageReceivedFromJetStream(const QByteArray &message)
{

    QString payload_type;

    QJsonDocument doc = QJsonDocument::fromJson(message);
    QJsonObject json_src = doc.object();
    QHash<QString, QString> commit_type_to;
    commit_type_to["c"] = "create";
    commit_type_to["u"] = "update";
    commit_type_to["d"] = "delete";

    if (doc.isNull() || json_src.isEmpty()) {
        qDebug().noquote() << "Invalid data";
        qDebug().noquote() << "message:" << message;
        emit errorOccured("InvalidData", "Unreadable JSON data.");
        m_webSocket.close();
    } else if (!json_src.contains("type") || !json_src.contains("did")
               || !json_src.contains("commit")) {
        qDebug().noquote() << "Unsupport data:" << message;
        // emit errorOccured("InvalidData", "Unanticipated data structure.");
        // m_webSocket.close();
    } else {
        payload_type = "#commit";

        QJsonObject json_src_commit = json_src.value("commit").toObject();
        QJsonObject json_dest;

        json_dest.insert("repo", json_src.value("did").toString());
        json_dest.insert("rev", json_src_commit.value("rev").toString());
        json_dest.insert("time",
                         QDateTime::fromMSecsSinceEpoch(
                                 static_cast<qint64>(json_src.value("time_us").toDouble() / 1000))
                                 .toString(Qt::ISODateWithMs));

        QJsonObject json_dest_commit;
        json_dest_commit.insert("$link", json_src_commit.value("cid").toString());
        json_dest.insert("commit", json_dest_commit);

        QJsonObject json_dest_op;
        QString commit_type = commit_type_to.value(json_src_commit.value("type").toString());
        json_dest_op.insert("action", commit_type);
        json_dest_op.insert("path",
                            QString("%1/%2").arg(json_src_commit.value("collection").toString(),
                                                 json_src_commit.value("rkey").toString()));
        if (commit_type == "delete") {
            json_dest_op.insert("cid", QJsonValue());
        } else {
            json_dest_op.insert("cid", json_dest_commit);
        }
        QJsonArray json_dest_ops;
        json_dest_ops.append(json_dest_op);
        json_dest.insert("ops", json_dest_ops);

        QJsonArray json_dest_blocks;
        if (json_src_commit.contains("record")) {
            QJsonObject json_dest_block;
            json_dest_block.insert("cid", json_src_commit.value("cid").toString());
            json_dest_block.insert("uri",
                                   QString("at://%1/%2/%3")
                                           .arg(json_src.value("did").toString(),
                                                json_src_commit.value("collection").toString(),
                                                json_src_commit.value("rkey").toString()));
            json_dest_block.insert("value", json_src_commit.value("record").toObject());
            json_dest_blocks.append(json_dest_block);
        }
        json_dest.insert("blocks", json_dest_blocks);

        emit received(payload_type, json_dest);
    }
}
}
