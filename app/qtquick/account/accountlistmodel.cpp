#include "accountlistmodel.h"
#include "extension/com/atproto/server/comatprotoservercreatesessionex.h"
#include "extension/com/atproto/server/comatprotoserverrefreshsessionex.h"
#include "extension/com/atproto/repo/comatprotorepogetrecordex.h"
#include "atprotocol/app/bsky/actor/appbskyactorgetprofile.h"
#include "extension/directory/plc/directoryplc.h"

#include <QByteArray>
#include <QSettings>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QJsonDocument>
#include <QNetworkInterface>

using AtProtocolInterface::AccountData;
using AtProtocolInterface::AccountStatus;
using AtProtocolInterface::AppBskyActorGetProfile;
using AtProtocolInterface::ComAtprotoRepoGetRecordEx;
using AtProtocolInterface::ComAtprotoServerCreateSessionEx;
using AtProtocolInterface::ComAtprotoServerRefreshSessionEx;
using AtProtocolInterface::DirectoryPlc;

AccountListModel::AccountListModel(QObject *parent) : QAbstractListModel { parent }
{
    m_roleTo[UuidRole] = AccountManager::AccountManagerRoles::UuidRole;
    m_roleTo[IsMainRole] = AccountManager::AccountManagerRoles::IsMainRole;
    m_roleTo[ServiceRole] = AccountManager::AccountManagerRoles::ServiceRole;
    m_roleTo[ServiceEndpointRole] = AccountManager::AccountManagerRoles::ServiceEndpointRole;
    m_roleTo[IdentifierRole] = AccountManager::AccountManagerRoles::IdentifierRole;
    m_roleTo[PasswordRole] = AccountManager::AccountManagerRoles::PasswordRole;
    m_roleTo[DidRole] = AccountManager::AccountManagerRoles::DidRole;
    m_roleTo[HandleRole] = AccountManager::AccountManagerRoles::HandleRole;
    m_roleTo[EmailRole] = AccountManager::AccountManagerRoles::EmailRole;
    m_roleTo[AccessJwtRole] = AccountManager::AccountManagerRoles::AccessJwtRole;
    m_roleTo[RefreshJwtRole] = AccountManager::AccountManagerRoles::RefreshJwtRole;
    m_roleTo[DisplayNameRole] = AccountManager::AccountManagerRoles::DisplayNameRole;
    m_roleTo[DescriptionRole] = AccountManager::AccountManagerRoles::DescriptionRole;
    m_roleTo[AvatarRole] = AccountManager::AccountManagerRoles::AvatarRole;
    m_roleTo[PostLanguagesRole] = AccountManager::AccountManagerRoles::PostLanguagesRole;
    m_roleTo[ThreadGateTypeRole] = AccountManager::AccountManagerRoles::ThreadGateTypeRole;
    m_roleTo[ThreadGateOptionsRole] = AccountManager::AccountManagerRoles::ThreadGateOptionsRole;
    m_roleTo[PostGateQuoteEnabledRole] =
            AccountManager::AccountManagerRoles::PostGateQuoteEnabledRole;
    m_roleTo[StatusRole] = AccountManager::AccountManagerRoles::StatusRole;
    m_roleTo[AuthorizedRole] = AccountManager::AccountManagerRoles::AuthorizedRole;

    connect(&m_timer, &QTimer::timeout, [=]() {
        AccountManager *manager = AccountManager::getInstance();
        for (int row = 0; row < manager->count(); row++) {
            refreshSession(row);
        }
    });
    m_timer.start(60 * 60 * 1000);

    AccountManager *manager = AccountManager::getInstance();

    connect(manager, &AccountManager::errorOccured, this, &AccountListModel::errorOccured);
    connect(manager, &AccountManager::updatedSession, this, &AccountListModel::updatedSession);
    connect(manager, &AccountManager::updatedAccount, this, &AccountListModel::updatedAccount);
    connect(manager, &AccountManager::countChanged, this, &AccountListModel::countChanged);
    connect(manager, &AccountManager::finished, this, &AccountListModel::finished);
    connect(manager, &AccountManager::allAccountsReadyChanged, this,
            &AccountListModel::allAccountsReadyChanged);

    // 各カラムがアカウント情報をAccountManagerから取得するようになれば↓の2つをQMLへ投げる必要はなくなる
    // dataChangedは必要
    connect(this, &AccountListModel::updatedSession, this, [=](const QString &uuid) {
        int row = manager->indexAt(uuid);
        if (row >= 0) {
            emit dataChanged(index(row), index(row));
        }
    });
    connect(this, &AccountListModel::updatedAccount, this, [=](const QString &uuid) {
        int row = manager->indexAt(uuid);
        if (row >= 0) {
            emit dataChanged(index(row), index(row));
        }
    });
}

AccountListModel::~AccountListModel()
{
    AccountManager *manager = AccountManager::getInstance();

    disconnect(manager, &AccountManager::errorOccured, this, &AccountListModel::errorOccured);
    disconnect(manager, &AccountManager::updatedSession, this, &AccountListModel::updatedSession);
    disconnect(manager, &AccountManager::updatedAccount, this, &AccountListModel::updatedAccount);
    disconnect(manager, &AccountManager::countChanged, this, &AccountListModel::countChanged);
    disconnect(manager, &AccountManager::finished, this, &AccountListModel::finished);
    disconnect(manager, &AccountManager::allAccountsReadyChanged, this,
               &AccountListModel::allAccountsReadyChanged);
}

int AccountListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return count();
}

QVariant AccountListModel::data(const QModelIndex &index, int role) const
{
    return item(index.row(), static_cast<AccountListModelRoles>(role));
}

QVariant AccountListModel::item(int row, AccountListModelRoles role) const
{
    if (row < 0 || row >= count())
        return QVariant();

    AccountManager *manager = AccountManager::getInstance();
    const AccountData account = manager->getAccount(manager->getUuid(row));

    if (role == UuidRole)
        return account.uuid;
    else if (role == IsMainRole)
        return account.is_main;
    else if (role == ServiceRole)
        return account.service;
    else if (role == ServiceEndpointRole)
        return account.service_endpoint;
    else if (role == IdentifierRole)
        return account.identifier;
    else if (role == PasswordRole)
        return account.password;
    else if (role == DidRole)
        return account.did;
    else if (role == HandleRole)
        return account.handle;
    else if (role == EmailRole)
        return account.email;
    else if (role == AccessJwtRole)
        return account.status == AccountStatus::Authorized ? account.accessJwt : QString();
    else if (role == RefreshJwtRole)
        return account.refreshJwt;
    else if (role == DisplayNameRole)
        return account.displayName;
    else if (role == DescriptionRole)
        return account.description;
    else if (role == AvatarRole)
        return account.avatar;

    else if (role == PostLanguagesRole)
        return account.post_languages;
    else if (role == ThreadGateTypeRole)
        return account.thread_gate_type;
    else if (role == ThreadGateOptionsRole)
        return account.thread_gate_options;

    else if (role == PostGateQuoteEnabledRole)
        return account.post_gate_quote_enabled;

    else if (role == StatusRole)
        return static_cast<int>(account.status);
    else if (role == AuthorizedRole)
        return account.status == AccountStatus::Authorized;

    return QVariant();
}

void AccountListModel::update(int row, AccountListModelRoles role, const QVariant &value)
{
    AccountManager::getInstance()->update(
            row, m_roleTo.value(role, AccountManager::AccountManagerRoles::UnknownRole), value);

    emit dataChanged(index(row), index(row));
}

void AccountListModel::updateAccount(const QString &service, const QString &identifier,
                                     const QString &password, const QString &did,
                                     const QString &handle, const QString &email,
                                     const QString &accessJwt, const QString &refreshJwt,
                                     const bool authorized)
{
    AccountManager *manager = AccountManager::getInstance();
    const QStringList uuids = manager->getUuids();

    bool updated = false;
    for (const auto &uuid : uuids) {
        int i = manager->indexAt(uuid);
        const AtProtocolInterface::AccountData account = manager->getAccount(uuid);
        if (account.service == service && account.identifier == identifier) {
            // update
            manager->updateAccount(uuid, service, identifier, password, did, handle, email,
                                   accessJwt, refreshJwt, authorized);
            updated = true;
            emit dataChanged(index(i), index(i));
        }
    }
    if (!updated) {
        // append
        beginInsertRows(QModelIndex(), count(), count());
        manager->updateAccount(QString(), service, identifier, password, did, handle, email,
                               accessJwt, refreshJwt, authorized);
        endInsertRows();
    }
}

void AccountListModel::removeAccount(int row)
{
    AccountManager *manager = AccountManager::getInstance();
    if (row < 0 || row >= manager->count())
        return;

    beginRemoveRows(QModelIndex(), row, row);
    manager->removeAccount(manager->getUuid(row));
    endRemoveRows();
}

void AccountListModel::updateAccountProfile(const QString &service, const QString &identifier)
{
    const QStringList uuids = AccountManager::getInstance()->getUuids();
    for (const auto &uuid : uuids) {
        AccountData account = AccountManager::getInstance()->getAccount(uuid);
        if (account.service == service && account.identifier == identifier) {
            AccountManager::getInstance()->getProfile(indexAt(uuid));
        }
    }
}

int AccountListModel::indexAt(const QString &uuid)
{
    return AccountManager::getInstance()->indexAt(uuid);
}

int AccountListModel::getMainAccountIndex() const
{
    return AccountManager::getInstance()->getMainAccountIndex();
}

void AccountListModel::setMainAccount(int row)
{
    AccountManager::getInstance()->setMainAccount(row);

    emit dataChanged(index(0), index(AccountManager::getInstance()->count() - 1));
}

void AccountListModel::refreshAccountSession(const QString &uuid)
{
    int row = indexAt(uuid);
    if (row < 0)
        return;
    refreshSession(row);
}

void AccountListModel::refreshAccountProfile(const QString &uuid)
{
    int row = indexAt(uuid);
    if (row < 0)
        return;
    getProfile(row);
}

void AccountListModel::save() const
{
    AccountManager::getInstance()->save();
}

void AccountListModel::load()
{
    AccountManager *manager = AccountManager::getInstance();

    if (manager->count() > 0) {
        beginRemoveRows(QModelIndex(), 0, count() - 1);
        manager->clear();
        endRemoveRows();
    }

    manager->load();
    beginInsertRows(QModelIndex(), 0, manager->count() - 1);
    endInsertRows();
}

QVariant AccountListModel::account(int row) const
{
    QString uuid = AccountManager::getInstance()->getUuid(row);
    if (uuid.isEmpty()) {
        return QVariant();
    } else {
        return QVariant::fromValue<AccountData>(AccountManager::getInstance()->getAccount(uuid));
    }
}

QHash<int, QByteArray> AccountListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[UuidRole] = "uuid";
    roles[IsMainRole] = "isMain";
    roles[ServiceRole] = "service";
    roles[ServiceEndpointRole] = "serviceEndpoint";
    roles[IdentifierRole] = "identifier";
    roles[PasswordRole] = "password";
    roles[DidRole] = "did";
    roles[HandleRole] = "handle";
    roles[EmailRole] = "email";
    roles[AccessJwtRole] = "accessJwt";
    roles[RefreshJwtRole] = "refreshJwt";
    roles[DisplayNameRole] = "displayName";
    roles[DescriptionRole] = "description";
    roles[AvatarRole] = "avatar";
    roles[PostLanguagesRole] = "postLanguages";
    roles[ThreadGateTypeRole] = "threadGateType";
    roles[ThreadGateOptionsRole] = "threadGateOptions";
    roles[PostGateQuoteEnabledRole] = "postGateQuoteEnabled";
    roles[StatusRole] = "status";
    roles[AuthorizedRole] = "authorized";

    return roles;
}

void AccountListModel::createSession(int row)
{
    AccountManager::getInstance()->createSession(row);
}

void AccountListModel::refreshSession(int row, bool initial)
{
    AccountManager::getInstance()->refreshSession(row, initial);
}

void AccountListModel::getProfile(int row)
{
    AccountManager::getInstance()->getProfile(row);
}

int AccountListModel::count() const
{
    return AccountManager::getInstance()->count();
}

bool AccountListModel::allAccountsReady() const
{
    return AccountManager::getInstance()->allAccountsReady();
}
