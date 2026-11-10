#ifndef JSONWEBTOKEN_H
#define JSONWEBTOKEN_H

#include <QString>

class Es256;

class JsonWebToken
{
public:
    // DPoP proof(RFC 9449)を生成する
    // access_tokenを指定するとathを付ける(Resource Serverへのリクエスト用)
    static QByteArray generate(const Es256 &key, const QString &endpoint, const QString &method,
                               const QString &nonce, const QString &access_token = QString());
};

#endif // JSONWEBTOKEN_H
