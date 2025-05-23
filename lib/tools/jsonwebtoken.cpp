#include "jsonwebtoken.h"
#include "es256.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QByteArray>
#include <QFile>
#include <QDebug>

inline QByteArray base64UrlEncode(const QByteArray &data)
{
    QByteArray encoded =
            data.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
    return encoded;
}

inline QJsonObject createJwk()
{
    QJsonObject jwk;

    QByteArray x_coord;
    QByteArray y_coord;
    Es256::getInstance()->getAffineCoordinates(x_coord, y_coord);

    if (!x_coord.isEmpty() && !y_coord.isEmpty()) {
        jwk["kty"] = "EC"; // Key Type
        jwk["crv"] = "P-256"; // Curve
        jwk["x"] = QString::fromUtf8(x_coord);
        jwk["y"] = QString::fromUtf8(y_coord);
    }

    return jwk;
}

QByteArray JsonWebToken::generate(const QString &endpoint, const QString &client_id,
                                  const QString &method, const QString &nonce)
{
    // ヘッダー
    QJsonObject header;
    header["alg"] = "ES256";
    header["typ"] = "dpop+jwt";
    header["jwk"] = createJwk();
    QByteArray headerJson = QJsonDocument(header).toJson(QJsonDocument::Compact);
    QByteArray headerBase64 = base64UrlEncode(headerJson);

    // ペイロード
    qint64 epoch = QDateTime::currentSecsSinceEpoch();
    QJsonObject payload;
    payload["iss"] = "tech/relog/hagoromo";
    payload["sub"] = client_id;
    payload["htu"] = endpoint;
    payload["htm"] = method;
    payload["exp"] = epoch + 60000;
    payload["jti"] = QString(QString::number(epoch).toUtf8().toBase64());
    payload["iat"] = epoch; // 発行時間
    if (!nonce.isEmpty()) {
        payload["nonce"] = nonce;
    }
    QByteArray payloadJson = QJsonDocument(payload).toJson(QJsonDocument::Compact);
    QByteArray payloadBase64 = base64UrlEncode(payloadJson);

    // 署名
    QByteArray message = headerBase64 + "." + payloadBase64;
    QByteArray signature = Es256::getInstance()->sign(message);
    QByteArray signatureBase64 = base64UrlEncode(signature);

    // JWTトークン
    QByteArray jwt = headerBase64 + "." + payloadBase64 + "." + signatureBase64;
    return jwt;
}
