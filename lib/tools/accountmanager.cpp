#include "accountmanager.h"
#include "encryption.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

using AtProtocolInterface::AccountData;
using AtProtocolInterface::AccountStatus;

class AccountManager::Private : public QObject
{
public:
    explicit Private(AccountManager *parent);
    ~Private();

    QJsonObject save() const;
    void load(const QJsonObject &object);

    AccountData getAccount() const;
    void updateAccount(const QString &uuid, const QString &service, const QString &identifier,
                       const QString &password, const QString &did, const QString &handle,
                       const QString &email, const QString &accessJwt, const QString &refreshJwt,
                       const QString &thread_gate_type, const AccountStatus status);

    void getProfile();

private:
    AccountManager *q;

    AccountData m_account;
    Encryption m_encryption;
};

AccountManager::Private::Private(AccountManager *parent)
{
    qDebug().noquote() << this << "AccountManager::Private()";
}

AccountManager::Private::~Private()
{
    qDebug().noquote() << this << "AccountManager::~Private()";
}

QJsonObject AccountManager::Private::save() const
{
    QJsonObject account_item;
    account_item["uuid"] = m_account.uuid;
    account_item["is_main"] = m_account.is_main;
    account_item["service"] = m_account.service;
    account_item["identifier"] = m_account.identifier;
    account_item["password"] = m_encryption.encrypt(m_account.password);
    account_item["refresh_jwt"] = m_encryption.encrypt(m_account.refreshJwt);

    if (!m_account.post_languages.isEmpty()) {
        QJsonArray post_langs;
        for (const auto &lang : m_account.post_languages) {
            post_langs.append(lang);
        }
        account_item["post_languages"] = post_langs;
    }

    if (m_account.thread_gate_type.isEmpty()) {
        account_item["thread_gate_type"] = "everybody";
    } else {
        account_item["thread_gate_type"] = m_account.thread_gate_type;
    }
    if (!m_account.thread_gate_options.isEmpty()) {
        QJsonArray thread_gate_options;
        for (const auto &option : m_account.thread_gate_options) {
            thread_gate_options.append(option);
        }
        account_item["thread_gate_options"] = thread_gate_options;
    }
    account_item["post_gate_quote_enabled"] = m_account.post_gate_quote_enabled;

    return account_item;
}

void AccountManager::Private::load(const QJsonObject &object)
{

    QString temp_refresh = object.value("refresh_jwt").toString();
    m_account.uuid = object.value("uuid").toString();
    m_account.is_main = object.value("is_main").toBool();
    m_account.service = object.value("service").toString();
    m_account.service_endpoint = m_account.service;
    m_account.identifier = object.value("identifier").toString();
    m_account.password = m_encryption.decrypt(object.value("password").toString());
    m_account.refreshJwt = m_encryption.decrypt(temp_refresh);
    m_account.handle = m_account.identifier;
    for (const auto &value : object.value("post_languages").toArray()) {
        m_account.post_languages.append(value.toString());
    }

    m_account.thread_gate_type = object.value("thread_gate_type").toString("everybody");
    if (m_account.thread_gate_type.isEmpty()) {
        m_account.thread_gate_type = "everybody";
    }
    for (const auto &value : object.value("thread_gate_options").toArray()) {
        m_account.thread_gate_options.append(value.toString());
    }
    m_account.post_gate_quote_enabled = object.value("post_gate_quote_enabled").toBool(true);

    if (temp_refresh.isEmpty()) {
        // createSession(m_accountList.count() - 1);
    } else {
        // refreshSession(m_accountList.count() - 1, true);
    }
}

AccountData AccountManager::Private::getAccount() const
{
    return m_account;
}

void AccountManager::Private::updateAccount(const QString &uuid, const QString &service,
                                            const QString &identifier, const QString &password,
                                            const QString &did, const QString &handle,
                                            const QString &email, const QString &accessJwt,
                                            const QString &refreshJwt,
                                            const QString &thread_gate_type,
                                            const AccountStatus status)
{
    m_account.uuid = uuid;
    m_account.service = service;
    m_account.identifier = identifier;
    m_account.password = password;
    m_account.did = did;
    m_account.handle = handle;
    m_account.email = email;
    m_account.accessJwt = accessJwt;
    m_account.refreshJwt = refreshJwt;
    m_account.thread_gate_type = thread_gate_type;
    m_account.status = status;
}

void AccountManager::Private::getProfile()
{
    //
}

AccountManager::AccountManager(QObject *parent) : QObject { parent }
{
    qDebug().noquote() << this << "AccountManager()";
}

AccountManager::~AccountManager()
{
    qDebug().noquote() << this << "~AccountManager()";
    clear();
}

AccountManager *AccountManager::getInstance()
{
    static AccountManager instance;
    return &instance;
}

void AccountManager::clear()
{
    for (const auto d : dList) {
        delete d;
    }
    dList.clear();
    dIndex.clear();
}

QJsonDocument AccountManager::save() const
{
    QJsonArray account_array;

    for (const auto d : dList) {
        account_array.append(d->save());
    }

    return QJsonDocument(account_array);
}

void AccountManager::load(QJsonDocument &doc)
{
    if (doc.isArray()) {
        bool has_main = false;
        for (const auto &item : doc.array()) {
            QString uuid = item.toObject().value("uuid").toString();
            if (!dIndex.contains(uuid)) {
                dList.append(new AccountManager::Private(this));
                dIndex[uuid] = dList.count() - 1;
            }
            dList.last()->load(item.toObject());
        }
        if (!has_main) {
            // mainになっているものがない
        }
    }
}

AccountData AccountManager::getAccount(const QString &uuid) const
{
    if (!dIndex.contains(uuid)) {
        return AccountData();
    }
    return dList.at(dIndex.value(uuid))->getAccount();
}

void AccountManager::updateAccount(const QString &service, const QString &identifier,
                                   const QString &password, const QString &did,
                                   const QString &handle, const QString &email,
                                   const QString &accessJwt, const QString &refreshJwt,
                                   const bool authorized)
{
    bool updated = false;
    for (const auto d : dList) {
        // for (const auto &uuid : d.keys()) {
        AccountData account = d->getAccount();
        if (account.service == service && account.identifier == identifier) {
            d->updateAccount(account.uuid, service, identifier, password, did, handle, email,
                             accessJwt, refreshJwt, account.thread_gate_type,
                             authorized ? AccountStatus::Authorized : AccountStatus::Unauthorized);
        }
    }
    if (!updated) {
        // append
        QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
        dList.append(new AccountManager::Private(this));
        dIndex[uuid] = dList.count() - 1;
        dList.last()->updateAccount(
                uuid, service, identifier, password, did, handle, email, accessJwt, refreshJwt,
                "everybody", authorized ? AccountStatus::Authorized : AccountStatus::Unauthorized);
    }
}

void AccountManager::removeAccount(const QString &uuid)
{
    if (!dIndex.contains(uuid)) {
        return;
    }
    int i = dIndex.value(uuid);
    delete dList.at(i);
    dList.removeAt(i);

    // refresh index
    dIndex.clear();
    for (int i = 0; i < dList.count(); i++) {
        dIndex[dList.at(i)->getAccount().uuid] = i;
    }
}

void AccountManager::updateAccountProfile(const QString &uuid)
{
    if (!dIndex.contains(uuid)) {
        return;
    }
    dList.at(dIndex.value(uuid))->getProfile();
}

int AccountManager::getMainAccountIndex() const
{
    return 0;
}

void AccountManager::setMainAccount(const QString &uuid)
{
    for (const auto d : qAsConst(dList)) {
        if (d->getAccount().uuid == uuid) {
            // true
        } else {
            // false
        }
    }
}

bool AccountManager::checkAllAccountsReady() const
{
    int ready_count = 0;
    for (const auto d : dList) {
        if (d->getAccount().status == AccountStatus::Authorized) {
            ready_count++;
        }
    }
    return (!dList.isEmpty() && dList.count() == ready_count);
}

QStringList AccountManager::getUuids() const
{
    QStringList uuids;
    for (const auto d : dList) {
        uuids.append(d->getAccount().uuid);
    }
    return uuids;
}
