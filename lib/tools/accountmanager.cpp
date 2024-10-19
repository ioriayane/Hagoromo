#include "accountmanager.h"
#include "encryption.h"
#include "extension/com/atproto/server/comatprotoservercreatesessionex.h"
#include "extension/com/atproto/server/comatprotoserverrefreshsessionex.h"
#include "extension/com/atproto/repo/comatprotorepogetrecordex.h"
#include "atprotocol/app/bsky/actor/appbskyactorgetprofile.h"
#include "atprotocol/com/atproto/repo/comatprotorepodescriberepo.h"
#include "extension/directory/plc/directoryplc.h"
#include "atprotocol/lexicons_func_unknown.h"
#include "tools/pinnedpostcache.h"
#include "common.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

using AtProtocolInterface::AccountData;
using AtProtocolInterface::AccountStatus;
using AtProtocolInterface::AppBskyActorGetProfile;
using AtProtocolInterface::ComAtprotoRepoDescribeRepo;
using AtProtocolInterface::ComAtprotoRepoGetRecordEx;
using AtProtocolInterface::ComAtprotoServerCreateSessionEx;
using AtProtocolInterface::ComAtprotoServerRefreshSessionEx;
using AtProtocolInterface::DirectoryPlc;

class AccountManager::Private : public QObject
{
public:
    explicit Private(AccountManager *parent);
    ~Private();

    QJsonObject save() const;
    void load(const QJsonObject &object);

    bool update(AccountManager::AccountManagerRoles role, const QVariant &value);

    AccountData getAccount() const;
    void updateAccount(const QString &uuid, const QString &service, const QString &identifier,
                       const QString &password, const QString &did, const QString &handle,
                       const QString &email, const QString &accessJwt, const QString &refreshJwt,
                       const QString &thread_gate_type, const AccountStatus status);

    void createSession();
    void refreshSession(bool initial = false);
    void getProfile();
    void getServiceEndpoint(const QString &did, const QString &service,
                            std::function<void(const QString &service_endpoint)> callback);
    void setMain(bool is);

private:
    AccountManager *q;

    AccountData m_account;
    Encryption m_encryption;
};

AccountManager::Private::Private(AccountManager *parent) : q(parent)
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
        createSession();
    } else {
        refreshSession(true);
    }
}

bool AccountManager::Private::update(AccountManagerRoles role, const QVariant &value)
{
    bool need_save = false;

    if (role == UuidRole)
        m_account.uuid = value.toString();
    else if (role == ServiceRole)
        m_account.service = value.toString();
    else if (role == ServiceEndpointRole)
        m_account.service_endpoint = value.toString();
    else if (role == IdentifierRole)
        m_account.identifier = value.toString();
    else if (role == PasswordRole)
        m_account.password = value.toString();
    else if (role == DidRole)
        m_account.did = value.toString();
    else if (role == HandleRole)
        m_account.handle = value.toString();
    else if (role == EmailRole)
        m_account.email = value.toString();
    else if (role == AccessJwtRole)
        m_account.accessJwt = value.toString();
    else if (role == RefreshJwtRole)
        m_account.refreshJwt = value.toString();

    else if (role == DisplayNameRole)
        m_account.displayName = value.toString();
    else if (role == DescriptionRole)
        m_account.description = value.toString();
    else if (role == AvatarRole)
        m_account.avatar = value.toString();

    else if (role == PostLanguagesRole) {
        m_account.post_languages = value.toStringList();
        need_save = true;
    } else if (role == ThreadGateTypeRole) {
        m_account.thread_gate_type = value.toString();
        need_save = true;
    } else if (role == ThreadGateOptionsRole) {
        m_account.thread_gate_options = value.toStringList();
        need_save = true;
    } else if (role == PostGateQuoteEnabledRole) {
        m_account.post_gate_quote_enabled = value.toBool();
    }
    return need_save;
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

void AccountManager::Private::createSession()
{
    ComAtprotoServerCreateSessionEx *session = new ComAtprotoServerCreateSessionEx(this);
    connect(session, &ComAtprotoServerCreateSessionEx::finished, [=](bool success) {
        //        qDebug() << session << session->service() << session->did() <<
        //        session->handle()
        //                 << session->email() << session->accessJwt() << session->refreshJwt();
        //        qDebug() << service << identifier << password;
        if (success) {
            qDebug() << "Create session" << session->did() << session->handle();
            m_account.did = session->did();
            m_account.handle = session->handle();
            m_account.email = session->email();
            m_account.accessJwt = session->accessJwt();
            m_account.refreshJwt = session->refreshJwt();

            // 詳細を取得
            getProfile();
        } else {
            qDebug() << "Fail createSession.";
            m_account.status = AccountStatus::Unauthorized;
            emit q->errorOccured(session->errorCode(), session->errorMessage());

            q->checkAllAccountsReady();
            if (q->allAccountTried()) {
                emit q->finished();
            }
        }
        session->deleteLater();
    });
    session->setAccount(m_account);
    session->createSession(m_account.identifier, m_account.password, QString());
}

void AccountManager::Private::refreshSession(bool initial)
{

    ComAtprotoServerRefreshSessionEx *session = new ComAtprotoServerRefreshSessionEx(this);
    connect(session, &ComAtprotoServerRefreshSessionEx::finished, [=](bool success) {
        if (success) {
            qDebug() << "Refresh session" << session->did() << session->handle()
                     << session->email();
            m_account.did = session->did();
            m_account.handle = session->handle();
            m_account.email = session->email();
            m_account.accessJwt = session->accessJwt();
            m_account.refreshJwt = session->refreshJwt();

            // 詳細を取得
            getProfile();
        } else {
            if (initial) {
                // 初期化時のみ（つまりloadから呼ばれたときだけは失敗したらcreateSessionで再スタート）
                qDebug() << "Initial refresh session fail.";
                m_account.status = AccountStatus::Unknown;
                createSession();
            } else {
                m_account.status = AccountStatus::Unauthorized;
                emit q->errorOccured(session->errorCode(), session->errorMessage());

                q->checkAllAccountsReady();
                if (q->allAccountTried()) {
                    emit q->finished();
                }
            }
        }
        session->deleteLater();
    });
    session->setAccount(m_account);
    session->refreshSession();
}

void AccountManager::Private::getProfile()
{

    getServiceEndpoint(m_account.did, m_account.service, [=](const QString &service_endpoint) {
        m_account.service_endpoint = service_endpoint;
        qDebug().noquote() << "Update service endpoint" << m_account.service << "->"
                           << m_account.service_endpoint;

        AppBskyActorGetProfile *profile = new AppBskyActorGetProfile(this);
        connect(profile, &AppBskyActorGetProfile::finished, [=](bool success) {
            if (success) {
                AtProtocolType::AppBskyActorDefs::ProfileViewDetailed detail =
                        profile->profileViewDetailed();
                qDebug() << "Update profile detailed" << detail.displayName << detail.description;
                m_account.displayName = detail.displayName;
                m_account.description = detail.description;
                m_account.avatar = detail.avatar;
                m_account.banner = detail.banner;
                m_account.status = AccountStatus::Authorized;

                q->save();

                emit q->updatedAccount(m_account.uuid);

                qDebug() << "Update pinned post" << detail.pinnedPost.uri;
                PinnedPostCache::getInstance()->update(m_account.did, detail.pinnedPost.uri);
            } else {
                emit q->errorOccured(profile->errorCode(), profile->errorMessage());
            }

            q->checkAllAccountsReady();
            if (q->allAccountTried()) {
                emit q->finished();
            }

            profile->deleteLater();
        });
        profile->setAccount(m_account);
        profile->getProfile(m_account.did);
    });
}

void AccountManager::Private::getServiceEndpoint(const QString &did, const QString &service,
                                                 std::function<void(const QString &)> callback)
{
    if (did.isEmpty()) {
        callback(service);
        return;
    }
    // if (!service.startsWith("https://bsky.social")) {
    //     callback(service);
    //     return;
    // }

    ComAtprotoRepoDescribeRepo *repo = new ComAtprotoRepoDescribeRepo(this);
    connect(repo, &ComAtprotoRepoDescribeRepo::finished, this, [=](bool success) {
        if (success) {
            AtProtocolType::DirectoryPlcDefs::DidDoc doc =
                    AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                            AtProtocolType::DirectoryPlcDefs::DidDoc>(repo->didDoc());
            if (!doc.service.isEmpty()) {
                callback(doc.service.first().serviceEndpoint);
            } else {
                callback(service);
            }
        } else {
            callback(service);
        }
        repo->deleteLater();
    });
    repo->setAccount(m_account);
    repo->describeRepo(did);
}

void AccountManager::Private::setMain(bool is)
{
    m_account.is_main = is;
}

AccountManager::AccountManager(QObject *parent) : QObject { parent }, m_allAccountsReady(false)
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
    emit countChanged();
}

void AccountManager::save() const
{
    QJsonArray account_array;

    for (const auto d : dList) {
        account_array.append(d->save());
    }

    Common::saveJsonDocument(QJsonDocument(account_array), QStringLiteral("account.json"));
}

void AccountManager::load()
{
    QJsonDocument doc = Common::loadJsonDocument(QStringLiteral("account.json"));

    if (doc.isArray()) {
        bool has_main = false;
        for (const auto &item : doc.array()) {
            QString uuid = item.toObject().value("uuid").toString();
            if (!dIndex.contains(uuid)) {
                dList.append(new AccountManager::Private(this));
                dIndex[uuid] = dList.count() - 1;

                emit countChanged();
            }
            dList.at(dIndex[uuid])->load(item.toObject());
        }
        if (!has_main && !dList.isEmpty()) {
            // mainになっているものがない
            dList.at(0)->setMain(true);
        }
    }
    if (dList.isEmpty()) {
        emit finished();
    }
}

void AccountManager::update(int row, AccountManagerRoles role, const QVariant &value)
{
    if (row < 0 || row >= count())
        return;

    if (dList.at(row)->update(role, value)) {
        save();
    }
}

AccountData AccountManager::getAccount(const QString &uuid) const
{
    if (!dIndex.contains(uuid)) {
        return AccountData();
    }
    return dList.at(dIndex.value(uuid))->getAccount();
}

QString AccountManager::updateAccount(const QString &uuid, const QString &service,
                                      const QString &identifier, const QString &password,
                                      const QString &did, const QString &handle,
                                      const QString &email, const QString &accessJwt,
                                      const QString &refreshJwt, const bool authorized)
{
    QString ret;
    if (!uuid.isEmpty() && dIndex.contains(uuid)) {
        AccountData account = dList.at(dIndex[uuid])->getAccount();
        dList.at(dIndex[uuid])
                ->updateAccount(account.uuid, service, identifier, password, did, handle, email,
                                accessJwt, refreshJwt, account.thread_gate_type,
                                authorized ? AccountStatus::Authorized
                                           : AccountStatus::Unauthorized);
        ret = uuid;
    } else {
        // append
        QString new_uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
        dList.append(new AccountManager::Private(this));
        dIndex[new_uuid] = dList.count() - 1;
        dList.last()->updateAccount(
                new_uuid, service, identifier, password, did, handle, email, accessJwt, refreshJwt,
                "everybody", authorized ? AccountStatus::Authorized : AccountStatus::Unauthorized);

        ret = new_uuid;
        emit countChanged();
    }
    save();
    checkAllAccountsReady();
    return ret;
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

    emit countChanged();
    checkAllAccountsReady();
}

void AccountManager::updateAccountProfile(const QString &uuid)
{
    if (!dIndex.contains(uuid)) {
        return;
    }
    dList.at(dIndex.value(uuid))->getProfile();
}

void AccountManager::updateServiceEndpoint(const QString &uuid, const QString &service_endpoint)
{
    update(indexAt(uuid), AccountManager::AccountManagerRoles::ServiceEndpointRole,
           service_endpoint);
}

int AccountManager::getMainAccountIndex() const
{
    if (dList.isEmpty())
        return -1;

    for (int i = 0; i < dList.count(); i++) {
        if (dList.at(i)->getAccount().is_main) {
            return i;
        }
    }
    return 0;
}

void AccountManager::setMainAccount(int row)
{
    if (row < 0 || row >= count())
        return;

    for (int i = 0; i < dList.count(); i++) {
        bool new_val = (row == i);
        if (dList.at(i)->getAccount().is_main != new_val) {
            dList.at(i)->setMain(new_val);
        }
    }

    save();
}

bool AccountManager::checkAllAccountsReady()
{
    int ready_count = 0;
    for (const auto d : qAsConst(dList)) {
        if (d->getAccount().status == AccountStatus::Authorized) {
            ready_count++;
        }
    }
    setAllAccountsReady(!dList.isEmpty() && dList.count() == ready_count);
    return allAccountsReady();
}

int AccountManager::indexAt(const QString &uuid)
{
    return dIndex.value(uuid, -1);
}

QStringList AccountManager::getUuids() const
{
    QStringList uuids;
    for (const auto d : dList) {
        uuids.append(d->getAccount().uuid);
    }
    return uuids;
}

QString AccountManager::getUuid(int row) const
{
    if (row < 0 || row >= count())
        return QString();
    return dList.at(row)->getAccount().uuid;
}

bool AccountManager::allAccountTried() const
{
    int count = 0;
    for (const auto d : dList) {
        if (d->getAccount().status != AccountStatus::Unknown) {
            count++;
        }
    }
    return (dList.count() == count);
}

bool AccountManager::allAccountsReady() const
{
    return m_allAccountsReady;
}

void AccountManager::setAllAccountsReady(bool newAllAccountsReady)
{
    if (m_allAccountsReady == newAllAccountsReady)
        return;
    m_allAccountsReady = newAllAccountsReady;
    emit allAccountsReadyChanged();
}

int AccountManager::count() const
{
    return dList.count();
}

void AccountManager::createSession(int row)
{
    if (row < 0 || row >= count())
        return;
    dList.at(row)->createSession();
}

void AccountManager::refreshSession(int row, bool initial)
{
    if (row < 0 || row >= count())
        return;
    dList.at(row)->refreshSession(initial);
}

void AccountManager::getProfile(int row)
{
    if (row < 0 || row >= count())
        return;
    dList.at(row)->getProfile();
}
