#include "accountlistmodel.h"
#include "common.h"
#include "atprotocol/com/atproto/server/comatprotoservercreatesession.h"
#include "atprotocol/com/atproto/server/comatprotoserverrefreshsession.h"
#include "atprotocol/app/bsky/actor/appbskyactorgetprofile.h"

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
using AtProtocolInterface::ComAtprotoServerCreateSession;
using AtProtocolInterface::ComAtprotoServerRefreshSession;

AccountListModel::AccountListModel(QObject *parent) : QAbstractListModel { parent }
{
    connect(&m_timer, &QTimer::timeout, [=]() {
        for (int row = 0; row < m_accountList.count(); row++) {
            refreshSession(row);
        }
    });
    m_timer.start(60 * 60 * 1000);

#ifndef HAGOROMO_UNIT_TEST
    load();
#endif
}

int AccountListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_accountList.count();
}

QVariant AccountListModel::data(const QModelIndex &index, int role) const
{
    return item(index.row(), static_cast<AccountListModelRoles>(role));
}

QVariant AccountListModel::item(int row, AccountListModelRoles role) const
{
    if (row < 0 || row >= m_accountList.count())
        return QVariant();

    if (role == UuidRole)
        return m_accountList.at(row).uuid;
    else if (role == IsMainRole)
        return m_accountList.at(row).is_main;
    else if (role == ServiceRole)
        return m_accountList.at(row).service;
    else if (role == ServiceEndpointRole)
        return m_accountList.at(row).service_endpoint;
    else if (role == IdentifierRole)
        return m_accountList.at(row).identifier;
    else if (role == PasswordRole)
        return m_accountList.at(row).password;
    else if (role == DidRole)
        return m_accountList.at(row).did;
    else if (role == HandleRole)
        return m_accountList.at(row).handle;
    else if (role == EmailRole)
        return m_accountList.at(row).email;
    else if (role == AccessJwtRole)
        return m_accountList.at(row).accessJwt;
    else if (role == RefreshJwtRole)
        return m_accountList.at(row).refreshJwt;
    else if (role == DisplayNameRole)
        return m_accountList.at(row).displayName;
    else if (role == DescriptionRole)
        return m_accountList.at(row).description;
    else if (role == AvatarRole)
        return m_accountList.at(row).avatar;

    else if (role == PostLanguagesRole)
        return m_accountList[row].post_languages;

    else if (role == StatusRole)
        return static_cast<int>(m_accountList.at(row).status);

    return QVariant();
}

void AccountListModel::update(int row, AccountListModelRoles role, const QVariant &value)
{
    if (row < 0 || row >= m_accountList.count())
        return;

    if (role == UuidRole)
        m_accountList[row].uuid = value.toString();
    else if (role == ServiceRole)
        m_accountList[row].service = value.toString();
    else if (role == ServiceEndpointRole)
        m_accountList[row].service_endpoint = value.toString();
    else if (role == IdentifierRole)
        m_accountList[row].identifier = value.toString();
    else if (role == PasswordRole)
        m_accountList[row].password = value.toString();
    else if (role == DidRole)
        m_accountList[row].did = value.toString();
    else if (role == HandleRole)
        m_accountList[row].handle = value.toString();
    else if (role == EmailRole)
        m_accountList[row].email = value.toString();
    else if (role == AccessJwtRole)
        m_accountList[row].accessJwt = value.toString();
    else if (role == RefreshJwtRole)
        m_accountList[row].refreshJwt = value.toString();

    else if (role == DisplayNameRole)
        m_accountList[row].displayName = value.toString();
    else if (role == DescriptionRole)
        m_accountList[row].description = value.toString();
    else if (role == AvatarRole)
        m_accountList[row].avatar = value.toString();

    else if (role == PostLanguagesRole) {
        m_accountList[row].post_languages = value.toStringList();
        save();
    }

    emit dataChanged(index(row), index(row));
}

void AccountListModel::updateAccount(const QString &service, const QString &service_endpoint,
                                     const QString &identifier, const QString &password,
                                     const QString &did, const QString &handle,
                                     const QString &email, const QString &accessJwt,
                                     const QString &refreshJwt, const bool authorized)
{
    bool updated = false;
    for (int i = 0; i < m_accountList.count(); i++) {
        if (m_accountList.at(i).service == service
            && m_accountList.at(i).identifier == identifier) {
            // update
            m_accountList[i].service_endpoint = service_endpoint;
            m_accountList[i].password = password;
            m_accountList[i].did = did;
            m_accountList[i].handle = handle;
            m_accountList[i].email = email;
            m_accountList[i].accessJwt = accessJwt;
            m_accountList[i].refreshJwt = refreshJwt;
            m_accountList[i].status =
                    authorized ? AccountStatus::Authorized : AccountStatus::Unauthorized;
            updated = true;
            emit dataChanged(index(i), index(i));
        }
    }
    if (!updated) {
        // append
        AtProtocolInterface::AccountData item;
        item.uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
        item.service = service;
        item.service_endpoint = service_endpoint;
        item.identifier = identifier;
        item.password = password;
        item.did = did;
        item.handle = handle;
        item.email = email;
        item.accessJwt = accessJwt;
        item.refreshJwt = refreshJwt;
        item.status = authorized ? AccountStatus::Authorized : AccountStatus::Unauthorized;

        beginInsertRows(QModelIndex(), count(), count());
        m_accountList.append(item);
        endInsertRows();

        emit countChanged();
    }

    save();
}

void AccountListModel::removeAccount(int row)
{
    if (row < 0 || row >= m_accountList.count())
        return;

    beginRemoveRows(QModelIndex(), row, row);
    m_accountList.removeAt(row);
    endRemoveRows();
    emit countChanged();

    save();
}

void AccountListModel::updateAccountProfile(const QString &service, const QString &identifier)
{
    for (int i = 0; i < m_accountList.count(); i++) {
        if (m_accountList.at(i).service == service && m_accountList.at(i).identifier == identifier
            && m_accountList.at(i).status == AccountStatus::Authorized) {
            getProfile(i);
        }
    }
}

int AccountListModel::indexAt(const QString &uuid)
{
    if (uuid.isEmpty())
        return -1;

    for (int i = 0; i < m_accountList.count(); i++) {
        if (m_accountList.at(i).uuid == uuid) {
            return i;
        }
    }
    return -1;
}

QString AccountListModel::getService(int row) const
{
    if (row < 0 || row >= m_accountList.count())
        return QString();

    if (m_accountList.at(row).service_endpoint.isEmpty()) {
        return m_accountList.at(row).service;
    } else {
        return m_accountList.at(row).service_endpoint;
    }
}

int AccountListModel::getMainAccountIndex() const
{
    if (m_accountList.isEmpty())
        return -1;

    for (int i = 0; i < m_accountList.count(); i++) {
        if (m_accountList.at(i).is_main) {
            return i;
        }
    }
    return 0;
}

void AccountListModel::setMainAccount(int row)
{
    if (row < 0 || row >= m_accountList.count())
        return;

    for (int i = 0; i < m_accountList.count(); i++) {
        bool new_val = (row == i);
        if (m_accountList.at(i).is_main != new_val) {
            m_accountList[i].is_main = new_val;
            emit dataChanged(index(i), index(i));
        }
    }

    save();
}

bool AccountListModel::allAccountsReady() const
{
    bool ready = true;
    for (const AccountData &item : qAsConst(m_accountList)) {
        if (item.status == AccountStatus::Unknown) {
            ready = false;
            break;
        }
    }
    return ready;
}

void AccountListModel::refreshAccountSession(const QString &uuid)
{
    int row = indexAt(uuid);
    if (row < 0)
        return;
    refreshSession(row);
}

void AccountListModel::save() const
{
    QSettings settings;

    QJsonArray account_array;
    for (const AccountData &item : m_accountList) {
        QJsonObject account_item;
        account_item["uuid"] = item.uuid;
        account_item["is_main"] = item.is_main;
        account_item["service"] = item.service;
        account_item["service_endpoint"] = item.service_endpoint;
        account_item["identifier"] = item.identifier;
        account_item["password"] = m_encryption.encrypt(item.password);
        account_item["refresh_jwt"] = m_encryption.encrypt(item.refreshJwt);

        if (!item.post_languages.isEmpty()) {
            QJsonArray post_langs;
            for (const auto &lang : item.post_languages) {
                post_langs.append(lang);
            }
            account_item["post_languages"] = post_langs;
        }
        account_array.append(account_item);
    }

    Common::saveJsonDocument(QJsonDocument(account_array), QStringLiteral("account.json"));
}

void AccountListModel::load()
{
    if (!m_accountList.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, count() - 1);
        m_accountList.clear();
        endRemoveRows();
        emit countChanged();
    }

    QJsonDocument doc = Common::loadJsonDocument(QStringLiteral("account.json"));

    if (doc.isArray()) {
        bool has_main = false;
        for (int i = 0; i < doc.array().count(); i++) {
            if (doc.array().at(i).isObject()) {
                AccountData item;
                QString temp_refresh = doc.array().at(i).toObject().value("refresh_jwt").toString();
                item.uuid = doc.array().at(i).toObject().value("uuid").toString();
                item.is_main = doc.array().at(i).toObject().value("is_main").toBool();
                item.service = doc.array().at(i).toObject().value("service").toString();
                item.service_endpoint =
                        doc.array().at(i).toObject().value("service_endpoint").toString();
                item.identifier = doc.array().at(i).toObject().value("identifier").toString();
                item.password = m_encryption.decrypt(
                        doc.array().at(i).toObject().value("password").toString());
                item.refreshJwt = m_encryption.decrypt(temp_refresh);
                for (const auto &value :
                     doc.array().at(i).toObject().value("post_languages").toArray()) {
                    item.post_languages.append(value.toString());
                }

                beginInsertRows(QModelIndex(), count(), count());
                m_accountList.append(item);
                endInsertRows();
                emit countChanged();

                if (temp_refresh.isEmpty()) {
                    createSession(m_accountList.count() - 1);
                } else {
                    refreshSession(m_accountList.count() - 1, true);
                }

                if (item.is_main) {
                    has_main = true;
                }
            }
        }
        if (!has_main && !m_accountList.isEmpty()) {
            // mainになっているものがない
            m_accountList[0].is_main = true;
        }
    }
}

QVariant AccountListModel::account(int row) const
{
    if (row < 0 || row >= m_accountList.count())
        return QVariant();
    return QVariant::fromValue<AccountData>(m_accountList.at(row));
}

QHash<int, QByteArray> AccountListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[UuidRole] = "uuid";
    roles[IsMainRole] = "isMain";
    roles[ServiceRole] = "service";
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
    roles[StatusRole] = "status";

    return roles;
}

void AccountListModel::createSession(int row)
{
    if (row < 0 || row >= m_accountList.count())
        return;

    ComAtprotoServerCreateSession *session = new ComAtprotoServerCreateSession(this);
    connect(session, &ComAtprotoServerCreateSession::finished, [=](bool success) {
        //        qDebug() << session << session->service() << session->did() << session->handle()
        //                 << session->email() << session->accessJwt() << session->refreshJwt();
        //        qDebug() << service << identifier << password;
        if (success) {
            qDebug() << "Create session" << session->did() << session->handle()
                     << session->serviceEndpoint();
            m_accountList[row].service_endpoint = session->serviceEndpoint();
            m_accountList[row].did = session->did();
            m_accountList[row].handle = session->handle();
            m_accountList[row].email = session->email();
            m_accountList[row].accessJwt = session->accessJwt();
            m_accountList[row].refreshJwt = session->refreshJwt();
            m_accountList[row].status = AccountStatus::Authorized;

            emit updatedSession(row, m_accountList[row].uuid);

            // 詳細を取得
            getProfile(row);
        } else {
            m_accountList[row].status = AccountStatus::Unauthorized;
            emit errorOccured(session->errorCode(), session->errorMessage());
        }
        emit dataChanged(index(row), index(row));
        session->deleteLater();
    });
    session->setAccount(m_accountList.at(row));
    session->create(m_accountList.at(row).identifier, m_accountList.at(row).password);
}

void AccountListModel::refreshSession(int row, bool initial)
{
    if (row < 0 || row >= m_accountList.count())
        return;

    ComAtprotoServerRefreshSession *session = new ComAtprotoServerRefreshSession(this);
    connect(session, &ComAtprotoServerRefreshSession::finished, [=](bool success) {
        if (success) {
            qDebug() << "Refresh session" << session->did() << session->handle();
            m_accountList[row].service_endpoint = session->serviceEndpoint();
            m_accountList[row].did = session->did();
            m_accountList[row].handle = session->handle();
            m_accountList[row].email = session->email();
            m_accountList[row].accessJwt = session->accessJwt();
            m_accountList[row].refreshJwt = session->refreshJwt();
            m_accountList[row].status = AccountStatus::Authorized;

            emit updatedSession(row, m_accountList[row].uuid);

            // 詳細を取得
            getProfile(row);
        } else {
            m_accountList[row].status = AccountStatus::Unauthorized;
            if (initial) {
                //初期化時のみ（つまりloadから呼ばれたときだけは失敗したらcreateSessionで再スタート）
                qDebug() << "Initial refresh session fail.";
                createSession(row);
            } else {
                emit errorOccured(session->errorCode(), session->errorMessage());
            }
        }
        emit dataChanged(index(row), index(row));
        session->deleteLater();
    });
    session->setAccount(m_accountList.at(row));
    session->refreshSession();
}

void AccountListModel::getProfile(int row)
{
    if (row < 0 || row >= m_accountList.count())
        return;

    AppBskyActorGetProfile *profile = new AppBskyActorGetProfile(this);
    profile->setAccount(m_accountList.at(row));
    connect(profile, &AppBskyActorGetProfile::finished, [=](bool success) {
        if (success) {
            AtProtocolType::AppBskyActorDefs::ProfileViewDetailed detail =
                    profile->profileViewDetailed();
            qDebug() << "Update profile detailed" << detail.displayName << detail.description;
            m_accountList[row].displayName = detail.displayName;
            m_accountList[row].description = detail.description;
            m_accountList[row].avatar = detail.avatar;
            m_accountList[row].banner = detail.banner;

            save();

            emit updatedAccount(row, m_accountList[row].uuid);
            emit dataChanged(index(row), index(row));
        } else {
            emit errorOccured(profile->errorCode(), profile->errorMessage());
        }
        profile->deleteLater();
    });
    profile->getProfile(m_accountList.at(row).did);
}

int AccountListModel::count() const
{
    return m_accountList.count();
}
