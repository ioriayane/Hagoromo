#ifndef ACCOUNTLISTMODEL_H
#define ACCOUNTLISTMODEL_H

#include "atprotocol/accessatprotocol.h"
#include "encryption.h"

#include <QAbstractListModel>
#include <QObject>
#include <QTimer>

class AccountListModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int count READ count NOTIFY countChanged CONSTANT)
    Q_PROPERTY(bool allAccountsReady READ allAccountsReady WRITE setAllAccountsReady NOTIFY
                       allAccountsReadyChanged FINAL)
public:
    explicit AccountListModel(QObject *parent = nullptr);

    // モデルで提供する項目のルールID的な（QML側へ公開するために大文字で始めること）
    enum AccountListModelRoles {
        ModelData = Qt::UserRole + 1,
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
        StatusRole,
        AuthorizedRole,
    };
    Q_ENUM(AccountListModelRoles)

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    Q_INVOKABLE QVariant item(int row, AccountListModel::AccountListModelRoles role) const;
    Q_INVOKABLE void update(int row, AccountListModel::AccountListModelRoles role,
                            const QVariant &value);

    Q_INVOKABLE void updateAccount(const QString &service, const QString &identifier,
                                   const QString &password, const QString &did,
                                   const QString &handle, const QString &email,
                                   const QString &accessJwt, const QString &refreshJwt,
                                   const bool authorized);
    Q_INVOKABLE void removeAccount(int row);
    Q_INVOKABLE void updateAccountProfile(const QString &service, const QString &identifier);
    Q_INVOKABLE int indexAt(const QString &uuid);
    Q_INVOKABLE int getMainAccountIndex() const;
    Q_INVOKABLE void setMainAccount(int row);
    Q_INVOKABLE bool checkAllAccountsReady();
    Q_INVOKABLE void refreshAccountSession(const QString &uuid);
    Q_INVOKABLE void refreshAccountProfile(const QString &uuid);

    Q_INVOKABLE void save() const;
    Q_INVOKABLE void load();

    Q_INVOKABLE QVariant account(int row) const;

    int count() const;
    bool allAccountsReady() const;
    void setAllAccountsReady(bool newAllAccountsReady);

signals:
    void errorOccured(const QString &code, const QString &message);
    void updatedSession(int row, const QString &uuid);
    void updatedAccount(int row, const QString &uuid);
    void countChanged();
    void finished();

    void allAccountsReadyChanged();

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    QList<AtProtocolInterface::AccountData> m_accountList;
    QVariant m_accountTemp;
    QTimer m_timer;
    Encryption m_encryption;
    bool m_allAccountsReady;

    QString appDataFolder() const;

    void createSession(int row);
    void refreshSession(int row, bool initial = false);
    void getProfile(int row);
    void getServiceEndpoint(const QString &did, const QString &service,
                            std::function<void(const QString &service_endpoint)> callback);
    bool allAccountTried() const;
};

#endif // ACCOUNTLISTMODEL_H
