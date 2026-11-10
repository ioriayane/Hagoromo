#ifndef BASE32_H
#define BASE32_H

#include <QByteArray>
#include <QString>

class Base32
{
public:
    static QString encode(const QByteArray &data, const bool with_padding = false);
    static QString encode_s(qint64 num);
};

#endif // BASE32_H
