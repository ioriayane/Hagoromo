#ifndef ACCESSATPROTOCOL_H
#define ACCESSATPROTOCOL_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QUrl>

namespace AtProtocolInterface {

enum class AccountStatus : int {
    Unknown,
    Unauthorized,
    Authorized,
};

struct AccountData
{
    QString uuid; //主にUIでカラムとの紐付け確認で使う

    QString service;
    QString identifier;
    QString password;

    QString did;
    QString handle;
    QString email;
    QString accessJwt;
    QString refreshJwt;

    QString displayName;
    QString description;
    QString avatar;
    QString banner;

    AccountStatus status = AccountStatus::Unknown;
};

class AccessAtProtocol : public QObject
{
    Q_OBJECT
public:
    explicit AccessAtProtocol(QObject *parent = nullptr);

    const AccountData &account() const;
    void setAccount(const AccountData &account);
    void setSession(const QString &did, const QString &handle, const QString &email,
                    const QString &access_jwt, const QString &refresh_jwt);
    QString service() const;
    void setService(const QString &newService);
    QString did() const;
    QString handle() const;
    QString email() const;
    QString accessJwt() const;
    QString refreshJwt() const;

signals:
    void finished(bool success);

public slots:

protected:
    void get(const QString &endpoint, const QUrlQuery &query, const bool with_auth_header = true);
    void post(const QString &endpoint, const QByteArray &json, const bool with_auth_header = true);
    void postWithImage(const QString &endpoint, const QString &path);

    virtual void parseJson(const QString reply_json) = 0;

private:
    QNetworkAccessManager m_manager;

    AccountData m_account;
};
}

Q_DECLARE_METATYPE(AtProtocolInterface::AccountStatus)
Q_DECLARE_METATYPE(AtProtocolInterface::AccountData)

#endif // ACCESSATPROTOCOL_H
