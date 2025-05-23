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

    enum AccountManagerRoles {
        UnknownRole,
        UuidRole,
        IsMainRole,
        ServiceRole,
        ServiceEndpointRole,
        IdentifierRole,
        PasswordRole,
        DidRole,
        HandleRole,
        EmailRole,
        AccessJwtRole,
        RefreshJwtRole,
        DisplayNameRole,
        DescriptionRole,
        AvatarRole,
        PostLanguagesRole,
        ThreadGateTypeRole,
        ThreadGateOptionsRole,
        PostGateQuoteEnabledRole,
        RealtimeFeedRulesRole,
        AllowedDirectMessageRole,
        StatusRole,
        AuthorizedRole,
    };

    void clear();
    void save() const;
    void load();

    void update(int row, AccountManager::AccountManagerRoles role, const QVariant &value);

    AtProtocolInterface::AccountData getAccount(const QString &uuid) const;
    QString updateAccount(const QString &uuid, const QString &service, const QString &identifier,
                          const QString &password, const QString &did, const QString &handle,
                          const QString &email, const QString &accessJwt, const QString &refreshJwt,
                          const bool authorized);
    void removeAccount(const QString &uuid);
    void updateAccountProfile(const QString &uuid);
    void updateServiceEndpoint(const QString &uuid, const QString &service_endpoint);
    void updateRealtimeFeedRule(const QString &uuid, const QString &name, const QString &condition);
    QList<AtProtocolInterface::RealtimeFeedRule> getRealtimeFeedRules(const QString &uuid);
    void removeRealtimeFeedRule(const QString &uuid, const QString &name);
    void loadPostInteractionSettings(const QString &uuid);
    void savePostInteractionSettings(const QString &uuid, const QString &thread_gate_type,
                                     const QStringList &thread_gate_options,
                                     const bool post_gate_quote_enabled);
    int getMainAccountIndex() const;
    void setMainAccount(int row);
    bool checkAllAccountsReady();
    int indexAt(const QString &uuid);

    void removeScope(const QString &uuid, AtProtocolInterface::AccountScope scope);

    QStringList getUuids() const;
    QString getUuid(int row) const;
    bool allAccountsReady() const;
    void setAllAccountsReady(bool newAllAccountsReady);
    int count() const;

    void createSession(int row);
    void refreshSession(int row, bool initial = false);
    void getProfile(int row);

signals:
    void errorOccured(const QString &code, const QString &message);
    void updatedAccount(const QString &uuid);
    void loadedPostInteractionSettings(const QString &uuid);
    void savedPostInteractionSettings(const QString &uuid);
    void countChanged();
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
