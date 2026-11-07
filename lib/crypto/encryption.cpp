#include "encryption.h"
#include "encryption_seed.h"

#include <QCryptographicHash>
#include <QDebug>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define ENCRYPT_IV QByteArray("wUn7qt@aVWtpjrr!")
#define ENCRYPT_V2_PREFIX QByteArrayLiteral("v2:")
#define ENCRYPT_V2_NONCE_SIZE 12
#define ENCRYPT_V2_TAG_SIZE 16

Encryption::Encryption()
{
    // v1とは別の鍵にする(同じ鍵を別の暗号モードで使わない)
    m_encryptKey = QCryptographicHash::hash(QByteArrayLiteral("hagoromo-encryption-v2:")
                                                    + QString(ENCRYPT_SEED).toUtf8(),
                                            QCryptographicHash::Sha256);
    m_legacyEncryptKey =
            QCryptographicHash::hash(QString(ENCRYPT_SEED).toUtf8(), QCryptographicHash::Sha256);
    m_legacyEncryptIv = ENCRYPT_IV;
}

QString Encryption::encrypt(const QString &data) const
{
    const QByteArray plain = data.toUtf8();

    // 同じ平文でも毎回違う暗号文になるように、nonceは暗号化のたびに作る
    QByteArray nonce(ENCRYPT_V2_NONCE_SIZE, '\0');
    if (RAND_bytes(reinterpret_cast<unsigned char *>(nonce.data()), nonce.size()) != 1) {
        qCritical() << "Failed to generate nonce for encryption";
        return QString();
    }

    QByteArray encrypted_data(plain.size(), '\0');
    QByteArray tag(ENCRYPT_V2_TAG_SIZE, '\0');
    int length = 0;
    int encrypted_data_size = 0;
    bool ret = false;

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (ctx == nullptr) {
    } else if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) {
    } else if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, nonce.size(), nullptr) != 1) {
    } else if (EVP_EncryptInit_ex(ctx, nullptr, nullptr,
                                  reinterpret_cast<const unsigned char *>(m_encryptKey.constData()),
                                  reinterpret_cast<const unsigned char *>(nonce.constData()))
               != 1) {
    } else if (!plain.isEmpty()
               && EVP_EncryptUpdate(
                          ctx, reinterpret_cast<unsigned char *>(encrypted_data.data()), &length,
                          reinterpret_cast<const unsigned char *>(plain.constData()), plain.size())
                       != 1) {
    } else {
        encrypted_data_size = length;
        if (EVP_EncryptFinal_ex(ctx,
                                reinterpret_cast<unsigned char *>(encrypted_data.data())
                                        + encrypted_data_size,
                                &length)
                    == 1
            && EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, tag.size(), tag.data()) == 1) {
            encrypted_data_size += length;
            ret = true;
        }
    }
    EVP_CIPHER_CTX_free(ctx);

    if (!ret) {
        qCritical() << "Failed to encrypt";
        return QString();
    }
    encrypted_data.resize(encrypted_data_size);
    return QString::fromUtf8(ENCRYPT_V2_PREFIX + (nonce + encrypted_data + tag).toBase64());
}

QString Encryption::decrypt(const QString &data) const
{
    const QByteArray encoded = data.toUtf8();
    if (encoded.startsWith(ENCRYPT_V2_PREFIX)) {
        return decryptV2(encoded.mid(ENCRYPT_V2_PREFIX.size()));
    }
    // 以前のバージョンで保存したもの
    return decryptLegacy(data);
}

QString Encryption::decryptV2(const QByteArray &data) const
{
    const QByteArray decoded = QByteArray::fromBase64(data);
    if (decoded.size() < ENCRYPT_V2_NONCE_SIZE + ENCRYPT_V2_TAG_SIZE) {
        return QString();
    }
    const QByteArray nonce = decoded.left(ENCRYPT_V2_NONCE_SIZE);
    QByteArray tag = decoded.right(ENCRYPT_V2_TAG_SIZE);
    const QByteArray encrypted_data = decoded.mid(
            ENCRYPT_V2_NONCE_SIZE, decoded.size() - ENCRYPT_V2_NONCE_SIZE - ENCRYPT_V2_TAG_SIZE);

    QByteArray decrypted_data(encrypted_data.size(), '\0');
    int length = 0;
    int decrypted_data_size = 0;
    bool ret = false;

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (ctx == nullptr) {
    } else if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) {
    } else if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, nonce.size(), nullptr) != 1) {
    } else if (EVP_DecryptInit_ex(ctx, nullptr, nullptr,
                                  reinterpret_cast<const unsigned char *>(m_encryptKey.constData()),
                                  reinterpret_cast<const unsigned char *>(nonce.constData()))
               != 1) {
    } else if (!encrypted_data.isEmpty()
               && EVP_DecryptUpdate(
                          ctx, reinterpret_cast<unsigned char *>(decrypted_data.data()), &length,
                          reinterpret_cast<const unsigned char *>(encrypted_data.constData()),
                          encrypted_data.size())
                       != 1) {
    } else {
        decrypted_data_size = length;
        // タグが一致しない(改ざん・鍵違い)ときはFinalが失敗する
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, tag.size(), tag.data()) == 1
            && EVP_DecryptFinal_ex(ctx,
                                   reinterpret_cast<unsigned char *>(decrypted_data.data())
                                           + decrypted_data_size,
                                   &length)
                    == 1) {
            decrypted_data_size += length;
            ret = true;
        }
    }
    EVP_CIPHER_CTX_free(ctx);

    if (!ret) {
        qWarning() << "Failed to decrypt";
        return QString();
    }
    decrypted_data.resize(decrypted_data_size);
    return QString::fromUtf8(decrypted_data);
}

QString Encryption::encryptLegacy(const QString &data) const
{
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                       reinterpret_cast<const unsigned char *>(m_legacyEncryptKey.constData()),
                       reinterpret_cast<const unsigned char *>(m_legacyEncryptIv.constData()));

    QByteArray encrypted_data;
    int block_size = EVP_CIPHER_CTX_block_size(ctx);
    encrypted_data.resize(data.toUtf8().size() + block_size);

    int encrypted_data_size = 0;
    EVP_EncryptUpdate(ctx, reinterpret_cast<unsigned char *>(encrypted_data.data()),
                      &encrypted_data_size,
                      reinterpret_cast<const unsigned char *>(data.toUtf8().constData()),
                      data.toUtf8().size());

    int final_size;
    EVP_EncryptFinal_ex(
            ctx, reinterpret_cast<unsigned char *>(encrypted_data.data() + encrypted_data_size),
            &final_size);
    encrypted_data_size += final_size;

    encrypted_data.resize(encrypted_data_size);
    EVP_CIPHER_CTX_free(ctx);

    return encrypted_data.toBase64();
}

QString Encryption::decryptLegacy(const QString &data) const
{
    const QByteArray &encrypted_data = QByteArray::fromBase64(data.toUtf8());

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                       reinterpret_cast<const unsigned char *>(m_legacyEncryptKey.constData()),
                       reinterpret_cast<const unsigned char *>(m_legacyEncryptIv.constData()));

    QByteArray decrypted_data;
    decrypted_data.resize(encrypted_data.size());

    int decrypted_data_size = 0;
    EVP_DecryptUpdate(ctx, reinterpret_cast<unsigned char *>(decrypted_data.data()),
                      &decrypted_data_size,
                      reinterpret_cast<const unsigned char *>(encrypted_data.constData()),
                      encrypted_data.size());

    int final_size;
    if (EVP_DecryptFinal_ex(
                ctx, reinterpret_cast<unsigned char *>(decrypted_data.data() + decrypted_data_size),
                &final_size)
        == 1) {
        decrypted_data_size += final_size;
    }

    decrypted_data.resize(decrypted_data_size);
    EVP_CIPHER_CTX_free(ctx);

    return QString::fromUtf8(decrypted_data);
}
