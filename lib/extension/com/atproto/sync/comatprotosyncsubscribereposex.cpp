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
    QElapsedTimer et;
    et.start();

    int offset = 0;
    bool is_error_frame = false;
    QString payload_type;
    while (offset < message.length()) {
        QCborValue cbor = QCborValue::fromCbor(message.mid(offset));
        const auto &cbor_map = cbor.toMap();
        offset += cbor.toCbor().length();

        // qDebug().noquote() << "decodeCbor" << offset;
        // qDebug().noquote() << QJsonDocument(json).toJson(QJsonDocument::Compact);

        if (cbor_map.contains(QStringLiteral("op"))) {
            // header
            // qDebug().noquote() << QJsonDocument(json).toJson(QJsonDocument::Compact);
            is_error_frame = (cbor_map.value("op").toInteger(-1) != 1);
            payload_type = cbor_map.value("t").toString();
        } else if (is_error_frame && cbor_map.contains(QStringLiteral("error"))) {
            // error payload
            qDebug().noquote() << QJsonDocument(cbor.toJsonValue().toObject()).toJson();
            emit errorOccured(cbor_map.value("error").toString(),
                              cbor_map.value("message").toString());
            close();
        } else if (cbor_map.contains(QStringLiteral("seq"))) {
            if (!m_payloadTypeList.contains(payload_type)) {
                // unknown payload type
                // skip
                qDebug().noquote() << "Unknown payload type" << payload_type;
                qDebug().noquote() << QJsonDocument(cbor.toJsonValue().toObject()).toJson();
            } else {
                // payload
                emit received(payload_type, cbor);
            }
        } else {
            // decode error
            qDebug().noquote() << QJsonDocument(cbor.toJsonValue().toObject()).toJson();
            emit errorOccured("DecodeError", "Unknown data format.");
            close();
        }
    }

    if (offset != message.length()) {
        qDebug().noquote() << "Invalid offset ?";
        emit errorOccured("InvalidDataSize",
                          "The size of the decoded data does not match the total.");
        close();
    }

    // qDebug().noquote() << "elapse" << et.nsecsElapsed();
    // qDebug().noquote() << "elapse" << et.elapsed();
}

int ComAtprotoSyncSubscribeReposEx::decodeCbor(const QByteArray &block, QJsonObject &json)
{
    QCborValue value = QCborValue::fromCbor(block);

    decodeCborObject(value, json);

    return value.toCbor().length();
}

bool ComAtprotoSyncSubscribeReposEx::decodeCborObject(const QCborValue &value, QJsonObject &parent)
{
    if (value.isNull())
        return false;

    switch (value.type()) {
    case QCborValue::Map:
        for (auto item = value.toMap().cbegin(); item != value.toMap().cend(); item++) {
            QVariant v = decodeCborValue(item.value());
            parent.insert(item.key().toString(), QJsonValue::fromVariant(v));
        }
        break;
    default:
        break;
    }
    return true;
}

QVariant ComAtprotoSyncSubscribeReposEx::decodeCborValue(const QCborValue &value)
{
    if (value.isNull())
        return QVariant();

    switch (value.type()) {
    case QCborValue::Integer:
        return value.toInteger();
    case QCborValue::ByteArray: {
        CarDecoder decoder;
        if (!decoder.setContent(value.toByteArray())) {
            return QString::fromUtf8(value.toByteArray());
        } else {
            QJsonArray records;
            for (const auto &cid : decoder.cids()) {
                QJsonObject record;
                if (decoder.type(cid) != "$car_address") {
                    record.insert("cid", cid);
                    record.insert("value", decoder.json(cid));
                    record.insert("uri", decoder.uri(cid));
                    records.append(record);
                }
            }
            return records;
        }
    }
    case QCborValue::String:
        return value.toString();
    case QCborValue::Array: {
        QList<QVariant> l;
        for (const auto &item : value.toArray()) {
            l.append(decodeCborValue(item));
        }
        return l;
    }
    case QCborValue::Map: {
        QJsonObject obj;
        decodeCborObject(value, obj);
        return obj;
    }
    case QCborValue::False:
        return false;
    case QCborValue::True:
        return true;
    case QCborValue::Null:
        break;
    case QCborValue::Undefined:
        break;
    case QCborValue::Double:
        return value.toDouble();
    case QCborValue::Invalid:
        break;
    case QCborValue::Tag:
        if (static_cast<int>(value.tag()) == 42 && value.taggedValue().isByteArray()) {
            // tag 42のときのbinaryをcidとしてデコードして良いのは42だからなのでここで処理する
            int offset = 0;
            QJsonObject obj;
            obj.insert("$link",
                       CarDecoder::decodeCid(value.taggedValue().toByteArray().mid(1), offset));
            return obj;
        }
        break;
    case QCborValue::DateTime:
        return value.toDateTime();
    case QCborValue::Url:
        return value.toUrl();
    case QCborValue::RegularExpression:
        return value.toRegularExpression().pattern();
    case QCborValue::Uuid:
        return value.toUuid().toString();
    default:
        break;
    }
    return QVariant();
}

}
