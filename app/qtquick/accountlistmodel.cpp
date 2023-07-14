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
    m_timer.start(15 * 60 * 1000);

    load();
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
    else if (role == ServiceRole)
        return m_accountList.at(row).service;
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

void AccountListModel::updateAccount(const QString &service, const QString &identifier,
                                     const QString &password, const QString &did,
                                     const QString &handle, const QString &email,
                                     const QString &accessJwt, const QString &refreshJwt,
                                     const bool authorized)
{
    bool updated = false;
    for (int i = 0; i < m_accountList.count(); i++) {
        if (m_accountList.at(i).service == service
            && m_accountList.at(i).identifier == identifier) {
            // update
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

void AccountListModel::save() const
{
    QSettings settings;

    QJsonArray account_array;
    for (const AccountData &item : m_accountList) {
        QJsonObject account_item;
        account_item["uuid"] = item.uuid;
        account_item["service"] = item.service;
        account_item["identifier"] = item.identifier;
        account_item["password"] = m_encryption.encrypt(item.password);

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
        for (int i = 0; i < doc.array().count(); i++) {
            if (doc.array().at(i).isObject()) {
                AccountData item;
                item.uuid = doc.array().at(i).toObject().value("uuid").toString();
                item.service = doc.array().at(i).toObject().value("service").toString();
                item.identifier = doc.array().at(i).toObject().value("identifier").toString();
                item.password = m_encryption.decrypt(
                        doc.array().at(i).toObject().value("password").toString());
                for (const auto &value :
                     doc.array().at(i).toObject().value("post_languages").toArray()) {
                    item.post_languages.append(value.toString());
                }

                beginInsertRows(QModelIndex(), count(), count());
                m_accountList.append(item);
                endInsertRows();
                emit countChanged();

                updateSession(m_accountList.count() - 1, item.service, item.identifier,
                              item.password);
            }
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

void AccountListModel::updateSession(int row, const QString &service, const QString &identifier,
                                     const QString &password)
{
    ComAtprotoServerCreateSession *session = new ComAtprotoServerCreateSession(this);
    session->setService(service);
    connect(session, &ComAtprotoServerCreateSession::finished, [=](bool success) {
        //        qDebug() << session << session->service() << session->did() << session->handle()
        //                 << session->email() << session->accessJwt() << session->refreshJwt();
        //        qDebug() << service << identifier << password;
        updateAccount(service, identifier, password, session->did(), session->handle(),
                      session->email(), session->accessJwt(), session->refreshJwt(), success);
        if (success) {
            emit appendedAccount(row);

            // 詳細を取得
            getProfile(row);
        } else {
            emit errorOccured(session->errorMessage());
        }
        bool all_finished = true;
        for (const AccountData &item : qAsConst(m_accountList)) {
            if (item.status == AccountStatus::Unknown) {
                all_finished = false;
                break;
            }
        }
        if (all_finished) {
            emit allFinished();
        }
        session->deleteLater();
    });
    session->create(identifier, password);
}

void AccountListModel::refreshSession(int row)
{
    if (row < 0 || row >= m_accountList.count())
        return;

    ComAtprotoServerRefreshSession *session = new ComAtprotoServerRefreshSession(this);
    session->setAccount(m_accountList.at(row));
    connect(session, &ComAtprotoServerRefreshSession::finished, [=](bool success) {
        if (success) {
            qDebug() << "Refresh session" << session->did() << session->handle();
            m_accountList[row].did = session->did();
            m_accountList[row].handle = session->handle();
            m_accountList[row].accessJwt = session->accessJwt();
            m_accountList[row].refreshJwt = session->refreshJwt();
            m_accountList[row].status = AccountStatus::Authorized;

            emit updatedAccount(row, m_accountList[row].uuid);
        } else {
            m_accountList[row].status = AccountStatus::Unauthorized;
            emit errorOccured(session->errorMessage());
        }
        emit dataChanged(index(row), index(row));
        session->deleteLater();
    });
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

            emit updatedAccount(row, m_accountList[row].uuid);
            emit dataChanged(index(row), index(row));
        } else {
            emit errorOccured(profile->errorMessage());
        }
        profile->deleteLater();
    });
    profile->getProfile(m_accountList.at(row).did);
}

int AccountListModel::count() const
{
    return m_accountList.count();
}
