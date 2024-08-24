#ifndef JSONWEBTOKEN_H
#define JSONWEBTOKEN_H

#include <QString>

class JsonWebToken
{
public:
    static QString generate(const QString &endpoint);

    static QByteArray sign(const QByteArray &data, const QString &privateKeyPath);
    static QByteArray base64UrlEncode(const QByteArray &data);
};

#endif // JSONWEBTOKEN_H
