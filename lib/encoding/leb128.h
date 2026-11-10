#ifndef LEB128_H
#define LEB128_H

#include <QByteArray>

class Leb128
{
public:
    static quint32 decode_u(const QByteArray &data, int &offset);
};

#endif // LEB128_H
