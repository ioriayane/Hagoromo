#include "cardecoder.h"
#include "leb128.h"
#include "base32.h"

#include <QCborValue>
#include <QCborMap>
#include <QCborArray>
#include <QJsonValue>
#include <QJsonArray>
#include <QDebug>

CarDecoder::CarDecoder() { }

bool CarDecoder::setContent(const QByteArray &content)
{
    m_offset = 0;
    m_content.clear();
    m_did.clear();
    m_cid.clear();
    m_type.clear();
    m_block.clear();
    m_json = QJsonObject();
    m_cid2uri.clear();

    if (content.isEmpty())
        return false;

    m_content = content;

    // decode header
    int offset = m_offset;
    m_lebSize = 0;
    m_dataSize = Leb128::decode_u(m_content.mid(offset), m_lebSize);
    // offset += m_lebSize;

    next();

    return decodeData();
}

bool CarDecoder::eof() const
{
    return (m_offset >= m_content.length());
}

bool CarDecoder::next()
{
    m_offset += m_lebSize + m_dataSize;
    return decodeData();
}

QString CarDecoder::cid() const
{
    return m_cid;
}

QString CarDecoder::type() const
{
    return m_type;
}

const QJsonObject &CarDecoder::json() const
{
    return m_json;
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

bool CarDecoder::decodeData()
{
    if (eof())
        return false;

    int offset = m_offset;

    m_lebSize = 0;
    m_dataSize = Leb128::decode_u(m_content.mid(offset), m_lebSize);
    offset += m_lebSize;

    int cid_size = 0;
    m_cid = decodeCid(m_content.mid(offset, m_dataSize), cid_size);
    offset += cid_size;

    m_block = m_content.mid(offset, m_dataSize - cid_size);

    return decodeCbor(m_block);
}

QString CarDecoder::decodeCid(const QByteArray &data, int &offset) const
{
    offset = 0;
    if (data.length() < 2) {
        return QString();
    } else if (data.at(0) == 0x12 && data.at(1) == 0x20) {
        // CIDv0
        qDebug() << "CIDv0 : Unknown format data.";
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

bool CarDecoder::decodeCbor(const QByteArray &block)
{
    QCborValue value = QCborValue::fromCbor(block);

    m_json = QJsonObject();
    decodeCborObject(value, m_json);

    if (m_json.contains("$type")) {
        m_type = m_json.value("$type").toString();
    } else {
        m_type = "$car_address";
        decodeCarAddress(m_json);
    }

    return true;
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
    case QCborValue::ByteArray:
        return QString::fromUtf8(value.toByteArray());
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
