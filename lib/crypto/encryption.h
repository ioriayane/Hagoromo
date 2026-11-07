#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <QObject>
#include <QString>

// 保存する資格情報の暗号化
//   v2(現在)  : "v2:" + base64(nonce(12) | 暗号文 | タグ(16))  AES-256-GCM、nonceは毎回ランダム
//   v1(従来)  : base64(暗号文)  AES-256-CBC、IV固定。復号(読み込み)のためだけに残す
class Encryption : public QObject
{
    Q_OBJECT

public:
    Encryption();

    Q_INVOKABLE QString encrypt(const QString &data) const;
    Q_INVOKABLE QString decrypt(const QString &data) const;

    // 従来形式(v1)の暗号化。移行のテスト用
    QString encryptLegacy(const QString &data) const;

private:
    QString decryptV2(const QByteArray &data) const;
    QString decryptLegacy(const QString &data) const;

    QByteArray m_encryptKey; // v2
    QByteArray m_legacyEncryptKey; // v1
    QByteArray m_legacyEncryptIv; // v1
};

#endif // ENCRYPTION_H
