#include "cardecoder.h"
#include "leb128.h"
#include "base32.h"

#include <QCborValue>
#include <QDebug>

CarDecoder::CarDecoder() { }

bool CarDecoder::setContent(const QByteArray &content)
{
    m_offset = 0;
    m_content.clear();
    m_cid.clear();
    m_block.clear();

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

    // qDebug() << "type" << value.type();

    return true;
}
