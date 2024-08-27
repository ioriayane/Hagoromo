#ifndef JSONWEBTOKEN_H
#define JSONWEBTOKEN_H

#include <QString>

class JsonWebToken
{
public:
    static QByteArray generate(const QString &endpoint);
};

#endif // JSONWEBTOKEN_H
