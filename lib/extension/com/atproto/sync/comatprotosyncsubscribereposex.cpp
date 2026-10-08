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
#include <QThread>

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
                emit errorOccurred("SslError", messages.join("\n"));
            });
    connect(&m_webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this,
            [this](QAbstractSocket::SocketError error) {
                // qDebug().noquote() << "error" << error;
                const QMetaObject &mo = QAbstractSocket::staticMetaObject;
                QMetaEnum metaEnum = mo.enumerator(mo.indexOfEnumerator("SocketError"));
                emit errorOccurred("SocketError", QString(metaEnum.valueToKey(error)));
            });
    connect(&m_webSocket, &QWebSocket::stateChanged, this,
            [this](QAbstractSocket::SocketState state) {
                qDebug().noquote() << "StateChanged:" << state;
                emit socketStateChanged(state);
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
    closeWebSocket();
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
            emit errorOccurred(json.value("error").toString(), json.value("message").toString());
            closeWebSocket();
        } else if (json.contains("seq")) {
            if (!m_payloadTypeList.contains(payload_type)) {
                // unknown payload type
                // skip
                qDebug().noquote() << "Unknown payload type" << payload_type;
                qDebug().noquote() << QJsonDocument(json).toJson();
            } else {
                // payload
                emit received(payload_type, json, message.length(), QByteArray());
            }
        } else {
            // decode error
            qDebug().noquote() << QJsonDocument(json).toJson();
            emit errorOccurred("DecodeError", "Unknown data format.");
            closeWebSocket();
        }
    }

    if (offset != message.length()) {
        qDebug().noquote() << "Invalid offset ?";
        emit errorOccurred("InvalidDataSize",
                           "The size of the decoded data does not match the total.");
        closeWebSocket();
    }
}

void ComAtprotoSyncSubscribeReposEx::messageReceivedFromJetStream(const QByteArray &message)
{
    QJsonDocument doc = QJsonDocument::fromJson(message);
    QJsonObject json_top = doc.object();

    if (doc.isNull() || json_top.isEmpty()) {
        qDebug().noquote() << "Invalid data";
        qDebug().noquote() << "message:" << message;
        emit errorOccurred("InvalidData", "Unreadable JSON data.");
        closeWebSocket();
        return;
    }

    const QString frame_type = json_top.value("$type").toString();
    if (frame_type == "error") {
        // Jetstream v2 closes the connection right after sending this frame.
        qDebug().noquote() << "JetStream error:" << json_top.value("error").toString()
                           << json_top.value("message").toString();
        emit errorOccurred(json_top.value("error").toString(),
                           json_top.value("message").toString());
        closeWebSocket();
        return;
    } else if (frame_type != "message") {
        qDebug().noquote() << "Unsupported JetStream frame:" << frame_type;
        return;
    }

    QJsonObject json_src = json_top.value("payload").toObject();
    if (!json_src.value("$type").toString().endsWith(QStringLiteral("#commit"))) {
        // #identity / #account / #sync / #info : not handled yet
        return;
    }

    emit received("#commit", convertJetStreamCommit(json_src), message.length(), message);
}

QJsonObject ComAtprotoSyncSubscribeReposEx::convertJetStreamMessage(const QByteArray &message)
{
    // JetStreamの受信データをFirehoseの形式に変換する(commit以外は空を返す)
    const QJsonObject json_top = QJsonDocument::fromJson(message).object();
    if (json_top.value("$type").toString() != "message") {
        return QJsonObject();
    }
    const QJsonObject json_src = json_top.value("payload").toObject();
    if (!json_src.value("$type").toString().endsWith(QStringLiteral("#commit"))) {
        return QJsonObject();
    }
    return convertJetStreamCommit(json_src);
}

QJsonObject ComAtprotoSyncSubscribeReposEx::convertJetStreamCommit(const QJsonObject &json_src)
{
    QJsonObject json_dest;

    json_dest.insert("repo", json_src.value("did").toString());
    json_dest.insert("rev", json_src.value("rev").toString());
    json_dest.insert("time", json_src.value("time").toString());
    json_dest.insert("seq", json_src.value("seq"));

    QJsonObject json_dest_commit;
    json_dest_commit.insert("$link", json_src.value("cid").toString());
    json_dest.insert("commit", json_dest_commit);

    QJsonObject json_dest_op;
    QString commit_op = json_src.value("operation").toString();
    json_dest_op.insert("action", commit_op);
    json_dest_op.insert("path",
                        QString("%1/%2").arg(json_src.value("collection").toString(),
                                             json_src.value("rkey").toString()));
    if (commit_op == "delete") {
        json_dest_op.insert("cid", QJsonValue());
    } else {
        json_dest_op.insert("cid", json_dest_commit);
    }
    QJsonArray json_dest_ops;
    json_dest_ops.append(json_dest_op);
    json_dest.insert("ops", json_dest_ops);

    QJsonArray json_dest_blocks;
    if (json_src.contains("record")) {
        QJsonObject json_dest_block;
        json_dest_block.insert("cid", json_src.value("cid").toString());
        json_dest_block.insert("uri",
                               QString("at://%1/%2/%3")
                                       .arg(json_src.value("did").toString(),
                                            json_src.value("collection").toString(),
                                            json_src.value("rkey").toString()));
        json_dest_block.insert("value", json_src.value("record").toObject());
        json_dest_blocks.append(json_dest_block);
    }
    json_dest.insert("blocks", json_dest_blocks);

    return json_dest;
}

void ComAtprotoSyncSubscribeReposEx::closeWebSocket()
{
    // m_webSocketは親を持たないのでmoveToThread()されたこのオブジェクトとは所属スレッドが異なる
    // 受信処理はこのオブジェクトのスレッドで動くため、所属スレッド以外からは直接操作しない
    if (QThread::currentThread() == m_webSocket.thread()) {
        m_webSocket.close();
    } else {
        QMetaObject::invokeMethod(
                &m_webSocket, [this]() { m_webSocket.close(); }, Qt::QueuedConnection);
    }
}

#ifdef QT_DEBUG // HAGOROMO_UNIT_TEST
void ComAtprotoSyncSubscribeReposEx::testMessageReceivedFromJetStream(const QByteArray &message)
{
    messageReceivedFromJetStream(message);
}
#endif
}
