#include "encryption.h"
#include "encryption_seed.h"

#include <QCryptographicHash>
#include <openssl/evp.h>

#define ENCRYPT_IV QByteArray("wUn7qt@aVWtpjrr!")

Encryption::Encryption()
{
    m_encryptKey =
            QCryptographicHash::hash(QString(ENCRYPT_SEED).toUtf8(), QCryptographicHash::Sha256);
    m_encryptIv = ENCRYPT_IV;
}

QString Encryption::encrypt(const QString &data) const
{

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                       reinterpret_cast<const unsigned char *>(m_encryptKey.constData()),
                       reinterpret_cast<const unsigned char *>(m_encryptIv.constData()));

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

QString Encryption::decrypt(const QString &data) const
{
    const QByteArray &encrypted_data = QByteArray::fromBase64(data.toUtf8());

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                       reinterpret_cast<const unsigned char *>(m_encryptKey.constData()),
                       reinterpret_cast<const unsigned char *>(m_encryptIv.constData()));

    QByteArray decrypted_data;
    decrypted_data.resize(encrypted_data.size());

    int decrypted_data_size = 0;
    EVP_DecryptUpdate(ctx, reinterpret_cast<unsigned char *>(decrypted_data.data()),
                      &decrypted_data_size,
                      reinterpret_cast<const unsigned char *>(encrypted_data.constData()),
                      encrypted_data.size());

    int final_size;
    EVP_DecryptFinal_ex(
            ctx, reinterpret_cast<unsigned char *>(decrypted_data.data() + decrypted_data_size),
            &final_size);
    decrypted_data_size += final_size;

    decrypted_data.resize(decrypted_data_size);
    EVP_CIPHER_CTX_free(ctx);

    return QString::fromUtf8(decrypted_data);
}
