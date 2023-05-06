#include "accountlistmodel.h"
#include "../atprotocol/comatprotoservercreatesession.h"

#include <QStandardPaths>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QByteArray>
#include <QSettings>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QJsonDocument>
#include <QDir>
#include <QNetworkInterface>
#include <openssl/evp.h>

AccountListModel::AccountListModel(QObject *parent) : QAbstractListModel { parent }
{

    QString seed = QStringLiteral("tech.relog.hagoromo.");
    const QList<QNetworkInterface> ni_list = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface &ni : ni_list) {
        if (!ni.hardwareAddress().isEmpty()) {
            seed += ni.hardwareAddress();
            break;
        }
    }
    m_encryptKey = QCryptographicHash::hash(seed.toUtf8(), QCryptographicHash::Sha256);
    m_encryptIv = QByteArray("wUn7qt@aVWtpjrr!");

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

    if (role == ServiceRole)
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
    else if (role == StatusRole)
        return static_cast<int>(m_accountList.at(row).status);

    return QVariant();
}

void AccountListModel::update(int row, AccountListModelRoles role, const QVariant &value)
{
    if (row < 0 || row >= m_accountList.count())
        return;

    if (role == ServiceRole)
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
        AccountData item;
        item.service = service;
        item.identifier = identifier;
        item.password = password;
        item.did = did;
        item.handle = handle;
        item.email = email;
        item.accessJwt = accessJwt;
        item.refreshJwt = refreshJwt;
        item.status = authorized ? AccountStatus::Authorized : AccountStatus::Unauthorized;

        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        m_accountList.append(item);
        endInsertRows();
    }

    save();
}

void AccountListModel::save() const
{
    QSettings settings;

    QJsonArray account_array;
    for (const AccountData &item : m_accountList) {
        QJsonObject account_item;
        account_item["service"] = item.service;
        account_item["identifier"] = item.identifier;
        account_item["password"] = encrypt(item.password);
        account_array.append(account_item);
    }

    QString folder = appDataFolder();
    QDir dir(folder);
    dir.mkpath(folder);
    QFile file(QString("%1/account.json").arg(folder));
    if (file.open(QFile::WriteOnly)) {
        QJsonDocument doc(account_array);
        file.write(doc.toJson());
        file.close();
    }
}

void AccountListModel::load()
{
    m_accountList.clear();

    QString folder = appDataFolder();
    QFile file(QString("%1/account.json").arg(folder));
    if (file.open(QFile::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();

        if (doc.isArray()) {
            for (int i = 0; i < doc.array().count(); i++) {
                if (doc.array().at(i).isObject()) {
                    AccountData item;
                    item.service = doc.array().at(i)["service"].toString();
                    item.identifier = doc.array().at(i)["identifier"].toString();
                    item.password = decrypt(doc.array().at(i)["password"].toString());

                    m_accountList.append(item);

                    updateSession(m_accountList.count() - 1, item.service, item.identifier,
                                  item.password);
                }
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

    roles[ServiceRole] = "service";
    roles[IdentifierRole] = "identifier";
    roles[PasswordRole] = "password";
    roles[DidRole] = "did";
    roles[HandleRole] = "handle";
    roles[EmailRole] = "email";
    roles[AccessJwtRole] = "accessJwt";
    roles[RefreshJwtRole] = "refreshJwt";
    roles[StatusRole] = "status";

    return roles;
}

QString AccountListModel::encrypt(const QString &data) const
{
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                       reinterpret_cast<const unsigned char *>(m_encryptKey.constData()),
                       reinterpret_cast<const unsigned char *>(m_encryptIv.constData()));

    QByteArray encrypted_data;
    int block_size = EVP_CIPHER_CTX_block_size(ctx);
    encrypted_data.resize(data.toUtf8().size() + block_size);

    int encrypted_data_size = 0;
    EVP_EncryptUpdate(ctx, reinterpret_cast<unsigned char *>(encrypted_data.data()),
                      &encrypted_data_size,
                      reinterpret_cast<const unsigned char *>(data.toUtf8().constData()),
                      data.toUtf8().size());

    int final_size;
    EVP_EncryptFinal_ex(
            ctx, reinterpret_cast<unsigned char *>(encrypted_data.data() + encrypted_data_size),
            &final_size);
    encrypted_data_size += final_size;

    encrypted_data.resize(encrypted_data_size);
    EVP_CIPHER_CTX_free(ctx);

    return encrypted_data.toBase64();
}

QString AccountListModel::decrypt(const QString &data) const
{
    const QByteArray &encrypted_data = QByteArray::fromBase64(data.toUtf8());

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                       reinterpret_cast<const unsigned char *>(m_encryptKey.constData()),
                       reinterpret_cast<const unsigned char *>(m_encryptIv.constData()));

    QByteArray decrypted_data;
    decrypted_data.resize(encrypted_data.size());

    int decrypted_data_size = 0;
    EVP_DecryptUpdate(ctx, reinterpret_cast<unsigned char *>(decrypted_data.data()),
                      &decrypted_data_size,
                      reinterpret_cast<const unsigned char *>(encrypted_data.constData()),
                      encrypted_data.size());

    int final_size;
    EVP_DecryptFinal_ex(
            ctx, reinterpret_cast<unsigned char *>(decrypted_data.data() + decrypted_data_size),
            &final_size);
    decrypted_data_size += final_size;

    decrypted_data.resize(decrypted_data_size);
    EVP_CIPHER_CTX_free(ctx);

    return QString::fromUtf8(decrypted_data);
}

QString AccountListModel::appDataFolder() const
{
    return QString("%1/%2/%3")
            .arg(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation))
            .arg(QCoreApplication::organizationName())
            .arg(QCoreApplication::applicationName());
}

void AccountListModel::updateSession(int row, const QString &service, const QString &identifier,
                                     const QString &password)
{
    ComAtprotoServerCreateSession *session = new ComAtprotoServerCreateSession();
    session->setService(service);
    connect(session, &ComAtprotoServerCreateSession::finished, [=](bool success) {
        //        qDebug() << session << session->service() << session->did() << session->handle()
        //                 << session->email() << session->accessJwt() << session->refreshJwt();
        //        qDebug() << service << identifier << password;
        updateAccount(service, identifier, password, session->did(), session->handle(),
                      session->email(), session->accessJwt(), session->refreshJwt(), success);
        if (success) {
            emit accountAppended(row);
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
