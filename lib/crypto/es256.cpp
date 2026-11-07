#include "es256.h"

#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/bn.h>
#include <openssl/core_names.h>
#include <openssl/objects.h>

#include <QDebug>

// P-256の座標と署名(r, s)のバイト長
#define ES256_COORDINATE_LENGTH 32

Es256::Es256() : m_pKey(nullptr) { }

Es256::~Es256()
{
    clear();
}

void Es256::clear()
{
    if (m_pKey != nullptr) {
        EVP_PKEY_free(m_pKey);
        m_pKey = nullptr;
    }
}

bool Es256::isValid() const
{
    return (m_pKey != nullptr);
}

bool Es256::generateKey()
{
    clear();

    EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr);
    if (pctx == nullptr) {
        return false;
    }

    if (EVP_PKEY_keygen_init(pctx) <= 0) {
        qWarning() << "Failed to initialize keygen context";
    } else if (EVP_PKEY_CTX_set_ec_paramgen_curve_nid(pctx, NID_X9_62_prime256v1) <= 0) {
        qWarning() << "Failed to set curve parameter";
    } else if (EVP_PKEY_keygen(pctx, &m_pKey) <= 0) {
        qWarning() << "Failed to generate EC key";
        m_pKey = nullptr;
    }
    EVP_PKEY_CTX_free(pctx);

    return isValid();
}

bool Es256::loadPrivateKeyPem(const QByteArray &pem)
{
    clear();
    if (pem.isEmpty()) {
        return false;
    }

    BIO *bio = BIO_new_mem_buf(pem.constData(), pem.length());
    if (bio == nullptr) {
        return false;
    }
    EVP_PKEY *pkey = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);

    if (pkey == nullptr) {
        qWarning() << "Failed to read private key";
        return false;
    }
    if (!isP256Key(pkey)) {
        qWarning() << "Private key is not P-256";
        EVP_PKEY_free(pkey);
        return false;
    }
    m_pKey = pkey;
    return true;
}

QByteArray Es256::privateKeyPem() const
{
    if (m_pKey == nullptr) {
        return QByteArray();
    }

    QByteArray pem;
    BIO *bio = BIO_new(BIO_s_mem());
    if (bio == nullptr) {
        return pem;
    }
    if (PEM_write_bio_PrivateKey(bio, m_pKey, nullptr, nullptr, 0, nullptr, nullptr) > 0) {
        char *data = nullptr;
        long length = BIO_get_mem_data(bio, &data);
        if (data != nullptr && length > 0) {
            pem = QByteArray(data, static_cast<int>(length));
        }
    } else {
        qWarning() << "Failed to write private key";
    }
    BIO_free(bio);
    return pem;
}

QByteArray Es256::sign(const QByteArray &data) const
{
    if (m_pKey == nullptr) {
        return QByteArray();
    }

    // ECDSA署名を生成
    QByteArray der_sig;
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    size_t sig_len = 0;
    bool ret = false;
    if (EVP_DigestSignInit(mdctx, nullptr, EVP_sha256(), nullptr, m_pKey) <= 0) {
        qWarning() << "Failed to initialize digest sign";
    } else if (EVP_DigestSignUpdate(mdctx, data.constData(), data.size()) <= 0) {
        qWarning() << "Failed to update digest sign";
    } else if (EVP_DigestSignFinal(mdctx, nullptr, &sig_len) <= 0) {
        qWarning() << "Failed to finalize digest sign 1";
    } else {
        der_sig.resize(static_cast<int>(sig_len));
        if (EVP_DigestSignFinal(mdctx, reinterpret_cast<unsigned char *>(der_sig.data()), &sig_len)
            <= 0) {
            qWarning() << "Failed to finalize digest sign 2";
        } else {
            der_sig.resize(static_cast<int>(sig_len));
            ret = true;
        }
    }
    EVP_MD_CTX_free(mdctx);
    if (!ret) {
        return QByteArray();
    }

    // Convert DER to IEEE P1363
    const unsigned char *temp = reinterpret_cast<const unsigned char *>(der_sig.constData());
    ECDSA_SIG *ec_sig = d2i_ECDSA_SIG(nullptr, &temp, der_sig.length());
    if (ec_sig == nullptr) {
        return QByteArray();
    }
    const BIGNUM *ec_sig_r = nullptr;
    const BIGNUM *ec_sig_s = nullptr;
    ECDSA_SIG_get0(ec_sig, &ec_sig_r, &ec_sig_s);
    const QByteArray rr = bn2ba(ec_sig_r, ES256_COORDINATE_LENGTH);
    const QByteArray ss = bn2ba(ec_sig_s, ES256_COORDINATE_LENGTH);
    ECDSA_SIG_free(ec_sig);

    if (rr.isEmpty() || ss.isEmpty()) {
        return QByteArray();
    }
    return rr + ss;
}

bool Es256::getAffineCoordinates(QByteArray &x_coord, QByteArray &y_coord) const
{
    x_coord.clear();
    y_coord.clear();
    if (m_pKey == nullptr) {
        return false;
    }

    BIGNUM *x = nullptr;
    BIGNUM *y = nullptr;
    if (EVP_PKEY_get_bn_param(m_pKey, OSSL_PKEY_PARAM_EC_PUB_X, &x) <= 0
        || EVP_PKEY_get_bn_param(m_pKey, OSSL_PKEY_PARAM_EC_PUB_Y, &y) <= 0) {
        qWarning() << "Failed to get affine coordinates";
    } else {
        // JWKの座標は先頭の0を省略せず32バイト固定
        x_coord = bn2ba(x, ES256_COORDINATE_LENGTH)
                          .toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
        y_coord = bn2ba(y, ES256_COORDINATE_LENGTH)
                          .toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
    }
    BN_free(x);
    BN_free(y);

    return (!x_coord.isEmpty() && !y_coord.isEmpty());
}

EVP_PKEY *Es256::pKey() const
{
    return m_pKey;
}

bool Es256::isP256Key(EVP_PKEY *pkey)
{
    if (pkey == nullptr || EVP_PKEY_base_id(pkey) != EVP_PKEY_EC) {
        return false;
    }
    char group_name[64] = { 0 };
    size_t group_name_length = 0;
    if (EVP_PKEY_get_utf8_string_param(pkey, OSSL_PKEY_PARAM_GROUP_NAME, group_name,
                                       sizeof(group_name), &group_name_length)
        <= 0) {
        return false;
    }
    const bool ret = (OBJ_txt2nid(group_name) == NID_X9_62_prime256v1);
    return ret;
}

QByteArray Es256::bn2ba(const BIGNUM *bn, int length)
{
    QByteArray ba;
    if (bn == nullptr || BN_num_bytes(bn) > length) {
        return ba;
    }
    ba.resize(length);
    if (BN_bn2binpad(bn, reinterpret_cast<unsigned char *>(ba.data()), length) != length) {
        ba.clear();
    }
    return ba;
}
