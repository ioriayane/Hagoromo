#include "jsonwebtoken.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QByteArray>
#include <QFile>
#include <QDebug>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

QString JsonWebToken::generate(const QString &endpoint)
{
    // ヘッダー
    QJsonObject header;
    header["alg"] = "ES256";
    header["typ"] = "dpop+jwt";
    QJsonObject jwk;
    jwk["kty"] = "EC";
    jwk["x"] = "TUZZd0VBWUhLb1pJemowQ0FRWUZLNEVFQUFvRFFnQUVzQlVDaE9vbVB2UXhnMlZkZ05RNnc2NE1LS3hhVmQ"
               "vRQ";
    jwk["y"] = "MVg2Y05QMU5DMFA4TzYrNVVsYm1LT1BGYlVsYk5FTGpFSVJtY3VraGlrMTFaSnp4UXY5dFJ3";
    jwk["crv"] = "P-256";
    header["jwk"] = jwk;
    QByteArray headerJson = QJsonDocument(header).toJson(QJsonDocument::Compact);
    QByteArray headerBase64 = JsonWebToken::base64UrlEncode(headerJson);

    // ペイロード
    QJsonObject payload;
    payload["sub"] = "1234567890"; // ユーザーIDなど
    payload["name"] = "John Doe";
    payload["iat"] = QDateTime::currentSecsSinceEpoch(); // 発行時間
    QByteArray payloadJson = QJsonDocument(payload).toJson(QJsonDocument::Compact);
    QByteArray payloadBase64 = JsonWebToken::base64UrlEncode(payloadJson);

    // 署名
    QByteArray message = headerBase64 + "." + payloadBase64;
    QByteArray signature = JsonWebToken::sign(message, "c:\\temp\\private_key.pem");
    QByteArray signatureBase64 = JsonWebToken::base64UrlEncode(signature);

    // JWTトークン
    QString jwt = headerBase64 + "." + payloadBase64 + "." + signatureBase64;
    return jwt;
}

QByteArray JsonWebToken::sign(const QByteArray &data, const QString &privateKeyPath)
{
    QByteArray signature;

    // OpenSSLで秘密鍵を読み込む
    FILE *fp = fopen(privateKeyPath.toStdString().c_str(), "r");
    if (!fp) {
        qWarning() << "Failed to open private key file";
        return signature;
    }

    EVP_PKEY *pkey = PEM_read_PrivateKey(fp, nullptr, nullptr, nullptr);
    fclose(fp);

    if (!pkey) {
        qWarning() << "Failed to read private key";
        return signature;
    }

    // ECDSA署名を生成
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (EVP_DigestSignInit(mdctx, nullptr, EVP_sha256(), nullptr, pkey) <= 0) {
        qWarning() << "Failed to initialize digest sign";
        EVP_MD_CTX_free(mdctx);
        EVP_PKEY_free(pkey);
        return signature;
    }

    if (EVP_DigestSignUpdate(mdctx, data.constData(), data.size()) <= 0) {
        qWarning() << "Failed to update digest sign";
        EVP_MD_CTX_free(mdctx);
        EVP_PKEY_free(pkey);
        return signature;
    }

    size_t sigLen;
    if (EVP_DigestSignFinal(mdctx, nullptr, &sigLen) <= 0) {
        qWarning() << "Failed to finalize digest sign";
        EVP_MD_CTX_free(mdctx);
        EVP_PKEY_free(pkey);
        return signature;
    }

    signature.resize(sigLen);
    if (EVP_DigestSignFinal(mdctx, reinterpret_cast<unsigned char *>(signature.data()), &sigLen)
        <= 0) {
        qWarning() << "Failed to finalize digest sign";
    }

    EVP_MD_CTX_free(mdctx);
    EVP_PKEY_free(pkey);

    return signature;
}

QByteArray JsonWebToken::base64UrlEncode(const QByteArray &data)
{
    QByteArray encoded = data.toBase64();
    encoded = encoded.replace('+', '-').replace('/', '_').replace("=", "");
    return encoded;
}
