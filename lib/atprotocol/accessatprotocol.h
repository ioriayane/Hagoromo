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
    bool is_main = false;

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

    QStringList post_languages; // BCP47形式で3つまで

    AccountStatus status = AccountStatus::Unknown;
};

class AtProtocolAccount : public QObject
{
    Q_OBJECT
public:
    explicit AtProtocolAccount(QObject *parent = nullptr);

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

private:
    AccountData m_account;
};

class AccessAtProtocol : public AtProtocolAccount
{
    Q_OBJECT
public:
    explicit AccessAtProtocol(QObject *parent = nullptr);

    QString replyJson() const;
    QString errorCode() const;
    QString errorMessage() const;
    QString cursor() const;
    void setCursor(const QString &newCursor);

signals:
    void finished(bool success);

public slots:

protected:
    void get(const QString &endpoint, const QUrlQuery &query, const bool with_auth_header = true);
    void post(const QString &endpoint, const QByteArray &json, const bool with_auth_header = true);
    void postWithImage(const QString &endpoint, const QString &path);

    virtual bool parseJson(bool success, const QString reply_json) = 0;
    bool checkReply(QNetworkReply *reply);

private:
    QNetworkAccessManager m_manager;

    QString m_replyJson;
    QString m_errorCode;
    QString m_errorMessage;
    QString m_cursor;
};
}

Q_DECLARE_METATYPE(AtProtocolInterface::AccountStatus)
Q_DECLARE_METATYPE(AtProtocolInterface::AccountData)

#endif // ACCESSATPROTOCOL_H
