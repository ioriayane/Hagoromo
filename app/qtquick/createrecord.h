#ifndef CREATERECORD_H
#define CREATERECORD_H

#include "../atprotocol/accessatprotocol.h"
#include <QObject>

class CreateRecord : public QObject
{
    Q_OBJECT

public:
    explicit CreateRecord(QObject *parent = nullptr);

    Q_INVOKABLE void setAccount(const QString &service, const QString &did, const QString &handle,
                                const QString &email, const QString &accessJwt,
                                const QString &refreshJwt);

    Q_INVOKABLE void post(const QString &text);

signals:
    void finished(bool success);

private:
    AtProtocolInterface::AccountData m_account;
};

#endif // CREATERECORD_H
