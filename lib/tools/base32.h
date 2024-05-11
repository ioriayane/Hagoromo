#ifndef BASE32_H
#define BASE32_H

#include <QByteArray>
#include <QString>

class Base32
{
public:
    static QString encode(const QByteArray &data, const bool with_padding = false);
};

#endif // BASE32_H
