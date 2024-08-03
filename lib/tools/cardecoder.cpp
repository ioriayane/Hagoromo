#include "cardecoder.h"
#include "leb128.h"
#include "base32.h"

#include <QCborValue>
#include <QCborMap>
#include <QCborArray>
#include <QJsonValue>
#include <QJsonArray>
#include <QDebug>

CarDecoder::CarDecoder(bool forFirehose) : m_forFirehose(forFirehose) { }

bool CarDecoder::setContent(const QByteArray &content)
{
    m_content.clear();
    m_did.clear();
    m_cids.clear();
    m_cid2uri.clear();

    if (content.isEmpty())
        return false;

    m_content = content;

    // decode header
    int offset = 0;
    int leb_size = 0;
    int data_size = Leb128::decode_u(m_content.mid(offset), leb_size);
    offset += leb_size;
    if (data_size <= 0 || leb_size <= 0 || (offset + data_size) >= m_content.length()) {
        return false;
    }
    QByteArray block = m_content.mid(offset, data_size);
    if (decodeCbor(block, "__header__") < 0) {
        return false;
    }

    offset += data_size;
    // decode data
    do {
        int t = decodeData(offset);
        if (t < 0)
            break;
        offset += t;
    } while (offset < m_content.length());

    return true;
}

QStringList CarDecoder::cids() const
{
    return m_cids;
}

QString CarDecoder::type(const QString &cid) const
{
    return m_cid2type.value(cid);
}

QJsonObject CarDecoder::json(const QString &cid) const
{
    return m_cid2Json.value(cid);
}

QJsonObject CarDecoder::headerJson() const
{
    return m_cid2Json.value("__header__");
}

QString CarDecoder::did() const
{
    return m_did;
}

QString CarDecoder::uri(const QString &cid) const
{
    if (!m_cid2uri.contains(cid))
        return QString();
    return QString("at://%1/%2").arg(did()).arg(m_cid2uri[cid]);
}

int CarDecoder::decodeData(int offset)
{
    int leb_size = 0;
    int data_size = Leb128::decode_u(m_content.mid(offset), leb_size);
    offset += leb_size;

    if (data_size <= 0 || leb_size <= 0 || data_size >= m_content.length()) {
        return m_content.length();
    }

    int cid_size = 0;
    QString cid = decodeCid(m_content.mid(offset, data_size), cid_size);
    if (cid.isEmpty())
        return m_content.length();
    m_cids.append(cid);
    offset += cid_size;

    QByteArray block = m_content.mid(offset, data_size - cid_size);

    if (decodeCbor(block, cid) < 0) {
        return m_content.length();
    }

    return leb_size + data_size;
}

QString CarDecoder::decodeCid(const QByteArray &data, int &offset) const
{
    offset = 0;
    if (data.length() < 2) {
        return QString();
    } else if (data.at(0) == 0x12 && data.at(1) == 0x20) {
        // CIDv0
        qDebug() << "CIDv0 : Unknown format data.";
        return QString();
    } else if (data.length() < 4) {
        return QString();
    } else {
        // CIDv1
        int cid_ver = static_cast<int>(data.at(0));
        int cid_codec = static_cast<int>(data.at(1));
        int hash_ver = static_cast<int>(data.at(2));
        int hash_size = static_cast<int>(data.at(3));

        offset = hash_size + 4;
        return QString("b" + Base32::encode(data.mid(0, offset)));
    }
}

int CarDecoder::decodeCbor(const QByteArray &block, const QString &cid)
{
    QCborValue value = QCborValue::fromCbor(block);

    QJsonObject json;
    decodeCborObject(value, json);

    m_cid2Json[cid] = json;

    if (json.contains("$type")) {
        m_cid2type[cid] = json.value("$type").toString();
    } else {
        m_cid2type[cid] = "$car_address";
        decodeCarAddress(json);
    }

    return value.toCbor().length();
}

bool CarDecoder::decodeCborObject(const QCborValue &value, QJsonObject &parent)
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

QVariant CarDecoder::decodeCborValue(const QCborValue &value)
{
    if (value.isNull())
        return QVariant();

    switch (value.type()) {
    case QCborValue::Integer:
        return value.toInteger();
    case QCborValue::ByteArray: {
        if (!m_forFirehose) {
            return QString::fromUtf8(value.toByteArray());
        } else {
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
            obj.insert("$link", decodeCid(value.taggedValue().toByteArray().mid(1), offset));
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

void CarDecoder::decodeCarAddress(const QJsonObject &json)
{
    if (json.isEmpty())
        return;

    if (json.contains("did")) {
        m_did = json.value("did").toString();
    } else {
        QString prev_k;
        for (const auto &item : json.value("e").toArray()) {
            int p = item.toObject().value("p").toInt(-1);
            QString k = item.toObject().value("k").toString();
            QString v = item.toObject().value("v").toObject().value("$link").toString();
            if (p > 0 && prev_k.length() > p) {
                k = prev_k.left(p) + k;
            }
            if (!k.isEmpty() && !v.isEmpty()) {
                m_cid2uri[v] = k;
            }
            prev_k = k;
        }
    }
}
