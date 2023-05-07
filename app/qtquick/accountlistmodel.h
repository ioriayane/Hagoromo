#ifndef ACCOUNTLISTMODEL_H
#define ACCOUNTLISTMODEL_H

#include "../atprotocol/accessatprotocol.h"

#include <QAbstractListModel>
#include <QObject>

class AccountListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit AccountListModel(QObject *parent = nullptr);

    // モデルで提供する項目のルールID的な（QML側へ公開するために大文字で始めること）
    enum AccountListModelRoles {
        ModelData = Qt::UserRole + 1,
        ServiceRole,
        IdentifierRole,
        PasswordRole,
        DidRole,
        HandleRole,
        EmailRole,
        AccessJwtRole,
        RefreshJwtRole,
        StatusRole,
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

    Q_INVOKABLE void save() const;
    Q_INVOKABLE void load();

    Q_INVOKABLE QVariant account(int row) const;

signals:
    void accountAppended(int row);
    void allFinished();

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    QList<AtProtocolInterface::AccountData> m_accountList;
    QVariant m_accountTemp;

    QByteArray m_encryptKey;
    QByteArray m_encryptIv;
    QString encrypt(const QString &data) const;
    QString decrypt(const QString &data) const;

    QString appDataFolder() const;

    void updateSession(int row, const QString &service, const QString &identifier,
                       const QString &password);
};

#endif // ACCOUNTLISTMODEL_H
