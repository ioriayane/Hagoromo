#ifndef ACCOUNTMANAGER_H
#define ACCOUNTMANAGER_H

#include "atprotocol/accessatprotocol.h"

#include <QHash>
#include <QObject>
#include <QJsonDocument>

class AccountManager : public QObject
{
    Q_OBJECT
    explicit AccountManager(QObject *parent = nullptr);
    ~AccountManager();

public:
    static AccountManager *getInstance();
    void clear();

    QJsonDocument save() const;
    void load(QJsonDocument &doc);

    AtProtocolInterface::AccountData getAccount(const QString &uuid) const;
    void updateAccount(const QString &service, const QString &identifier, const QString &password,
                       const QString &did, const QString &handle, const QString &email,
                       const QString &accessJwt, const QString &refreshJwt, const bool authorized);

    QStringList getUuids() const;
signals:

private:
    class Private;
    QHash<QString, Private *> d;
    Q_DISABLE_COPY_MOVE(AccountManager)
};

#endif // ACCOUNTMANAGER_H
