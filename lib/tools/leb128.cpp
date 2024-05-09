#include "leb128.h"

quint32 Leb128::decode_u(const QByteArray &data, int &offset)
{
    quint32 result = 0;
    int shift = 0;
    quint8 byte;

    do {
        byte = data.at(offset++);
        result |= (byte & 0x7F) << shift;
        shift += 7;
    } while (byte & 0x80);

    return result;
}
