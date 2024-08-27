#ifndef ES256_H
#define ES256_H

#include <QByteArray>

#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/bn.h>

class Es256
{
    explicit Es256();
    ~Es256();

public:
    static Es256 *getInstance();

    void clear();
    void loadKey();
    QByteArray sign(const QByteArray &data);
    void getAffineCoordinates(QByteArray &x_coord, QByteArray &y_coord);
    EVP_PKEY *pKey() const;

private:
    void createPrivateKey(const QString &path);
    QByteArray bn2ba(const BIGNUM *bn);

    EVP_PKEY *m_pKey;
};

#endif // ES256_H
