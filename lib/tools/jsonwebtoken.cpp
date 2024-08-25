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
#include <openssl/bn.h>

QByteArray base64UrlEncode(const QByteArray &data)
{
    QByteArray encoded =
            data.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
    return encoded;
}

QByteArray bn2ba(const BIGNUM *bn)
{
    QByteArray ba;
    ba.resize(BN_num_bytes(bn));
    BN_bn2bin(bn, reinterpret_cast<unsigned char *>(ba.data()));
    return ba;
}

QJsonObject createJwk(EVP_PKEY *pkey)
{
    QJsonObject jwk;

    // ECキーの抽出
    EC_KEY *ec_key = EVP_PKEY_get1_EC_KEY(pkey);
    if (!ec_key) {
        qWarning() << "Failed to get EC key from EVP_PKEY";
        return jwk;
    }

    const EC_GROUP *group = EC_KEY_get0_group(ec_key);
    const EC_POINT *point = EC_KEY_get0_public_key(ec_key);

    // X, Y座標の抽出
    BIGNUM *x = BN_new();
    BIGNUM *y = BN_new();
    if (!EC_POINT_get_affine_coordinates_GFp(group, point, x, y, nullptr)) {
        qWarning() << "Failed to get affine coordinates";
        BN_free(x);
        BN_free(y);
        return jwk;
    }

    // X, Y座標をBase64URLエンコード
    QByteArray xCoord = base64UrlEncode(
            QByteArray::fromRawData(reinterpret_cast<const char *>(BN_bn2hex(x)), BN_num_bytes(x)));
    QByteArray yCoord = base64UrlEncode(
            QByteArray::fromRawData(reinterpret_cast<const char *>(BN_bn2hex(y)), BN_num_bytes(y)));

    jwk["kty"] = "EC"; // Key Type
    jwk["crv"] = "P-256"; // Curve
    jwk["x"] = QString::fromUtf8(xCoord);
    jwk["y"] = QString::fromUtf8(yCoord);

    BN_free(x);
    BN_free(y);

    return jwk;
}

QByteArray JsonWebToken::generate(const QString &endpoint)
{
    // OpenSSLで秘密鍵を読み込む
    FILE *fp = fopen("c:\\temp\\private_key.pem", "r");
    if (!fp) {
        qWarning() << "Failed to open private key file";
        return QByteArray();
    }
    EVP_PKEY *pkey = PEM_read_PrivateKey(fp, nullptr, nullptr, nullptr);
    fclose(fp);
    if (!pkey) {
        qWarning() << "Failed to read private key";
        return QByteArray();
    }

    // JWKを生成
    QJsonObject jwk = createJwk(pkey);
    EVP_PKEY_free(pkey);

    // ヘッダー
    QJsonObject header;
    header["alg"] = "ES256";
    header["typ"] = "dpop+jwt";
    header["jwk"] = jwk;
    QByteArray headerJson = QJsonDocument(header).toJson(QJsonDocument::Compact);
    QByteArray headerBase64 = base64UrlEncode(headerJson);

    // ペイロード
    QJsonObject payload;
    payload["sub"] = "1234567890"; // ユーザーIDなど
    payload["name"] = "John Doe";
    payload["iat"] = QDateTime::currentSecsSinceEpoch(); // 発行時間
    QByteArray payloadJson = QJsonDocument(payload).toJson(QJsonDocument::Compact);
    QByteArray payloadBase64 = base64UrlEncode(payloadJson);

    // 署名
    QByteArray message = headerBase64 + "." + payloadBase64;
    QByteArray signature = JsonWebToken::sign(message, "c:\\temp\\private_key.pem");
    QByteArray signatureBase64 = base64UrlEncode(signature);

    // JWTトークン
    QByteArray jwt = headerBase64 + "." + payloadBase64 + "." + signatureBase64;
    return jwt;
}

QByteArray JsonWebToken::sign(const QByteArray &data, const QString &privateKeyPath)
{
    QByteArray signature;

    // OpenSSLで秘密鍵を読み込む
    FILE *fp = fopen(privateKeyPath.toStdString().c_str(), "r");
    if (!fp) {
        qWarning() << "Failed to open private key file";
        return QByteArray();
    }

    EVP_PKEY *pkey = PEM_read_PrivateKey(fp, nullptr, nullptr, nullptr);
    fclose(fp);

    if (!pkey) {
        qWarning() << "Failed to read private key";
        return QByteArray();
    }

    // ECDSA署名を生成
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (EVP_DigestSignInit(mdctx, nullptr, EVP_sha256(), nullptr, pkey) <= 0) {
        qWarning() << "Failed to initialize digest sign";
        EVP_MD_CTX_free(mdctx);
        EVP_PKEY_free(pkey);
        return QByteArray();
    }

    if (EVP_DigestSignUpdate(mdctx, data.constData(), data.size()) <= 0) {
        qWarning() << "Failed to update digest sign";
        EVP_MD_CTX_free(mdctx);
        EVP_PKEY_free(pkey);
        return QByteArray();
    }

    size_t sigLen;
    if (EVP_DigestSignFinal(mdctx, nullptr, &sigLen) <= 0) {
        qWarning() << "Failed to finalize digest sign";
        EVP_MD_CTX_free(mdctx);
        EVP_PKEY_free(pkey);
        return QByteArray();
    }

    signature.resize(sigLen);
    if (EVP_DigestSignFinal(mdctx, reinterpret_cast<unsigned char *>(signature.data()), &sigLen)
        <= 0) {
        qWarning() << "Failed to finalize digest sign";
        return QByteArray();
    }

    // Convert DER to IEEE P1363
    const int ec_sig_len = 64; // ES256のときの値
    const unsigned char *temp = reinterpret_cast<const unsigned char *>(signature.constData());
    ECDSA_SIG *ec_sig = d2i_ECDSA_SIG(NULL, &temp, signature.length());
    if (ec_sig == NULL) {
        return QByteArray();
    }

    const BIGNUM *ec_sig_r = NULL;
    const BIGNUM *ec_sig_s = NULL;
    ECDSA_SIG_get0(ec_sig, &ec_sig_r, &ec_sig_s);

    QByteArray rr = bn2ba(ec_sig_r);
    QByteArray ss = bn2ba(ec_sig_s);

    if (rr.size() > (ec_sig_len / 2) || ss.size() > (ec_sig_len / 2)) {
        return QByteArray();
    }
    rr.insert(0, ec_sig_len / 2 - rr.size(), '\0');
    ss.insert(0, ec_sig_len / 2 - ss.size(), '\0');

    EVP_MD_CTX_free(mdctx);
    EVP_PKEY_free(pkey);

    return rr + ss;
}
