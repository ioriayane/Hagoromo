#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <QObject>
#include <QString>

class Encryption : public QObject
{
    Q_OBJECT

public:
    Encryption();

    Q_INVOKABLE QString encrypt(const QString &data) const;
    Q_INVOKABLE QString decrypt(const QString &data) const;

private:
    QByteArray m_encryptKey;
    QByteArray m_encryptIv;
};

#endif // ENCRYPTION_H
