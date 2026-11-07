#include "jsonwebtoken.h"
#include "es256.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QByteArray>
#include <QCryptographicHash>
#include <QDateTime>
#include <QRandomGenerator>
#include <QUrl>

inline QByteArray base64UrlEncode(const QByteArray &data)
{
    QByteArray encoded =
            data.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
    return encoded;
}

inline QJsonObject createJwk(const Es256 &key)
{
    QJsonObject jwk;

    QByteArray x_coord;
    QByteArray y_coord;
    if (key.getAffineCoordinates(x_coord, y_coord)) {
        jwk["kty"] = "EC"; // Key Type
        jwk["crv"] = "P-256"; // Curve
        jwk["x"] = QString::fromUtf8(x_coord);
        jwk["y"] = QString::fromUtf8(y_coord);
    }

    return jwk;
}

QByteArray JsonWebToken::generate(const Es256 &key, const QString &endpoint, const QString &method,
                                  const QString &nonce, const QString &access_token)
{
    if (!key.isValid()) {
        return QByteArray();
    }

    // ヘッダー
    QJsonObject header;
    header["alg"] = "ES256";
    header["typ"] = "dpop+jwt";
    header["jwk"] = createJwk(key);
    QByteArray headerJson = QJsonDocument(header).toJson(QJsonDocument::Compact);
    QByteArray headerBase64 = base64UrlEncode(headerJson);

    // ペイロード
    // 同一秒内の再送でも重複しないようにランダムな値にする
    QByteArray jti;
    for (int i = 0; i < 16; i++) {
        jti.append(static_cast<char>(QRandomGenerator::system()->bounded(256)));
    }
    QJsonObject payload;
    payload["jti"] = QString::fromUtf8(base64UrlEncode(jti));
    payload["htm"] = method;
    // htuはクエリとフラグメントを除く
    payload["htu"] = QUrl(endpoint).adjusted(QUrl::RemoveQuery | QUrl::RemoveFragment).toString();
    payload["iat"] = QDateTime::currentSecsSinceEpoch(); // 発行時間
    if (!nonce.isEmpty()) {
        payload["nonce"] = nonce;
    }
    if (!access_token.isEmpty()) {
        payload["ath"] = QString::fromUtf8(base64UrlEncode(
                QCryptographicHash::hash(access_token.toUtf8(), QCryptographicHash::Sha256)));
    }
    QByteArray payloadJson = QJsonDocument(payload).toJson(QJsonDocument::Compact);
    QByteArray payloadBase64 = base64UrlEncode(payloadJson);

    // 署名
    QByteArray message = headerBase64 + "." + payloadBase64;
    QByteArray signature = key.sign(message);
    if (signature.isEmpty()) {
        return QByteArray();
    }
    QByteArray signatureBase64 = base64UrlEncode(signature);

    // JWTトークン
    QByteArray jwt = headerBase64 + "." + payloadBase64 + "." + signatureBase64;
    return jwt;
}
