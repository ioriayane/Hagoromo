#include "es256.h"

#include <QCoreApplication>
#include <QStandardPaths>
#include <QString>
#include <QFile>
#include <QDebug>

Es256::Es256() : m_pKey(nullptr)
{
    qDebug().noquote() << this << "Es256()";
}

Es256::~Es256()
{
    qDebug().noquote() << this << "~Es256()";
}

Es256 *Es256::getInstance()
{
    static Es256 instance;
    return &instance;
}

void Es256::clear()
{
    if (m_pKey != nullptr) {
        EVP_PKEY_free(m_pKey);
        m_pKey = nullptr;
    }
}

QByteArray Es256::sign(const QByteArray &data)
{
    loadKey();
    if (m_pKey == nullptr) {
        return QByteArray();
    }

    QByteArray der_sig;

    // ECDSA署名を生成
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (EVP_DigestSignInit(mdctx, nullptr, EVP_sha256(), nullptr, m_pKey) <= 0) {
        qWarning() << "Failed to initialize digest sign";
        EVP_MD_CTX_free(mdctx);
        // EVP_PKEY_free(pkey);
        return QByteArray();
    }

    if (EVP_DigestSignUpdate(mdctx, data.constData(), data.size()) <= 0) {
        qWarning() << "Failed to update digest sign";
        EVP_MD_CTX_free(mdctx);
        // EVP_PKEY_free(pkey);
        return QByteArray();
    }

    size_t sigLen;
    if (EVP_DigestSignFinal(mdctx, nullptr, &sigLen) <= 0) {
        qWarning() << "Failed to finalize digest sign 1";
        EVP_MD_CTX_free(mdctx);
        // EVP_PKEY_free(pkey);
        return QByteArray();
    }

    der_sig.resize(sigLen);
    if (EVP_DigestSignFinal(mdctx, reinterpret_cast<unsigned char *>(der_sig.data()), &sigLen)
        <= 0) {
        qWarning() << "Failed to finalize digest sign 2";
        return QByteArray();
    }

    // Convert DER to IEEE P1363
    const int ec_sig_len = 64; // ES256のときの値
    const unsigned char *temp = reinterpret_cast<const unsigned char *>(der_sig.constData());
    ECDSA_SIG *ec_sig = d2i_ECDSA_SIG(NULL, &temp, der_sig.length());
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

    return rr + ss;
}

void Es256::getAffineCoordinates(QByteArray &x_coord, QByteArray &y_coord)
{
    loadKey();

    // ECキーの抽出
    EC_KEY *ec_key = EVP_PKEY_get1_EC_KEY(m_pKey);
    if (!ec_key) {
        qWarning() << "Failed to get EC key from EVP_PKEY";
    }

    const EC_GROUP *group = EC_KEY_get0_group(ec_key);
    const EC_POINT *point = EC_KEY_get0_public_key(ec_key);

    // X, Y座標の抽出
    BIGNUM *x = BN_new();
    BIGNUM *y = BN_new();
    if (!EC_POINT_get_affine_coordinates_GFp(group, point, x, y, nullptr)) {
        qWarning() << "Failed to get affine coordinates";
        x_coord.clear();
        y_coord.clear();
    } else {
        // X, Y座標をBase64URLエンコード
        x_coord = QByteArray::fromRawData(reinterpret_cast<const char *>(BN_bn2hex(x)),
                                          BN_num_bytes(x))
                          .toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
        y_coord = QByteArray::fromRawData(reinterpret_cast<const char *>(BN_bn2hex(y)),
                                          BN_num_bytes(y))
                          .toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
    }
    BN_free(x);
    BN_free(y);
}

void Es256::loadKey()
{
    if (m_pKey != nullptr) {
        return;
    }

    QString path =
            QString("%1/%2/%3%4/private_key.pem")
                    .arg(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation))
                    .arg(QCoreApplication::organizationName())
                    .arg(QCoreApplication::applicationName())
                    .arg(
#if defined(HAGOROMO_UNIT_TEST)
                            QStringLiteral("_unittest")
#elif defined(QT_DEBUG)
                            QStringLiteral("_debug")
#else
                            QString()
#endif
                    );

    if (!QFile::exists(path)) {
        createPrivateKey(path);
    } else {
        FILE *fp = fopen(path.toStdString().c_str(), "r");
        if (!fp) {
            qWarning() << "Failed to open private key file";
        } else {
            m_pKey = PEM_read_PrivateKey(fp, nullptr, nullptr, nullptr);
            fclose(fp);
        }
    }
}

void Es256::createPrivateKey(const QString &path)
{
    if (m_pKey != nullptr) {
        return;
    }

    EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr);
    if (pctx == nullptr) {
        return;
    }

    if (EVP_PKEY_keygen_init(pctx) <= 0) {
        qWarning() << "Failed to initialize keygen context";
    } else if (EVP_PKEY_CTX_set_ec_paramgen_curve_nid(pctx, NID_X9_62_prime256v1) <= 0) {
        qWarning() << "Failed to set curve parameter";
    } else if (EVP_PKEY_keygen(pctx, &m_pKey) <= 0) {
        qWarning() << "Failed to generate EC key";
        m_pKey = nullptr;
    } else {
        FILE *fp = fopen(path.toStdString().c_str(), "w");
        if (!fp) {
            qWarning() << "Failed to open private key file";
        } else {
            if (PEM_write_PrivateKey(fp, m_pKey, nullptr, nullptr, 0, nullptr, nullptr) <= 0) {
                qWarning() << "Failed to write private key to file";
            }
            fclose(fp);
        }
    }
    EVP_PKEY_CTX_free(pctx);
}

QByteArray Es256::bn2ba(const BIGNUM *bn)
{
    QByteArray ba;
    ba.resize(BN_num_bytes(bn));
    BN_bn2bin(bn, reinterpret_cast<unsigned char *>(ba.data()));
    return ba;
}

EVP_PKEY *Es256::pKey() const
{
    return m_pKey;
}
