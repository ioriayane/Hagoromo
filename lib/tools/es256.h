#ifndef ES256_H
#define ES256_H

#include <QByteArray>

// 利用側にOpenSSLのインクルードパスを要求しないよう前方宣言にとどめる
typedef struct evp_pkey_st EVP_PKEY;
typedef struct bignum_st BIGNUM;

// DPoP用のES256(P-256)の鍵
// 鍵はセッションごとに生成し、セッションと一緒に保存・復元する
class Es256
{
public:
    explicit Es256();
    ~Es256();

    Es256(const Es256 &) = delete;
    Es256 &operator=(const Es256 &) = delete;

    void clear();
    bool isValid() const;
    bool generateKey();
    bool loadPrivateKeyPem(const QByteArray &pem);
    QByteArray privateKeyPem() const;

    QByteArray sign(const QByteArray &data) const;
    bool getAffineCoordinates(QByteArray &x_coord, QByteArray &y_coord) const;
    EVP_PKEY *pKey() const;

private:
    static bool isP256Key(EVP_PKEY *pkey);
    static QByteArray bn2ba(const BIGNUM *bn, int length);

    EVP_PKEY *m_pKey;
};

#endif // ES256_H
