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

    Q_PROPERTY(bool allAccountsReady READ allAccountsReady WRITE setAllAccountsReady NOTIFY
                       allAccountsReadyChanged FINAL)
public:
    static AccountManager *getInstance();
    void clear();

    QJsonDocument save() const;
    void load(QJsonDocument &doc);

    AtProtocolInterface::AccountData getAccount(const QString &uuid) const;
    void updateAccount(const QString &service, const QString &identifier, const QString &password,
                       const QString &did, const QString &handle, const QString &email,
                       const QString &accessJwt, const QString &refreshJwt, const bool authorized);
    void removeAccount(const QString &uuid);
    void updateAccountProfile(const QString &uuid);
    int getMainAccountIndex() const;
    void setMainAccount(const QString &uuid);
    bool checkAllAccountsReady();
    int indexAt(const QString &uuid);

    QStringList getUuids() const;
    bool allAccountsReady() const;
    void setAllAccountsReady(bool newAllAccountsReady);

signals:
    void errorOccured(const QString &code, const QString &message);
    void updatedSession(const QString &uuid);
    void updatedAccount(const QString &uuid);
    void finished();
    void allAccountsReadyChanged();

private:
    class Private;
    QList<Private *> dList;
    QHash<QString, int> dIndex;
    Q_DISABLE_COPY_MOVE(AccountManager)

    bool allAccountTried() const;
    bool m_allAccountsReady;
};

#endif // ACCOUNTMANAGER_H
