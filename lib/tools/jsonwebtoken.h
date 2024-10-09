#ifndef JSONWEBTOKEN_H
#define JSONWEBTOKEN_H

#include <QString>

class JsonWebToken
{
public:
    static QByteArray generate(const QString &endpoint, const QString &client_id,
                               const QString &method, const QString &nonce);
};

#endif // JSONWEBTOKEN_H
