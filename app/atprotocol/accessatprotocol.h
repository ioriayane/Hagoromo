#ifndef ACCESSATPROTOCOL_H
#define ACCESSATPROTOCOL_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QUrl>

class AccessAtProtocol : public QObject
{
    Q_OBJECT
public:
    explicit AccessAtProtocol(QObject *parent = nullptr);

    void setSession(const QString &did, const QString &handle, const QString &email,
                    const QString &access_jwt, const QString &refresh_jwt);
    QString service() const;
    void setService(const QString &newService);
    QString did() const;
    void setDid(const QString &newDid);
    QString handle() const;
    void setHandle(const QString &newHandle);
    QString email() const;
    void setEmail(const QString &newEmail);
    QString accessJwt() const;
    void setAccessJwt(const QString &newAccessJwt);
    QString refreshJwt() const;
    void setRefreshJwt(const QString &newRefreshJwt);

signals:
    void finished(bool success);

public slots:

protected:
    void get(const QString &endpoint, const QUrlQuery &query);
    void post(const QString &endpoint, const QByteArray &json, const bool with_auth_header = true);

    virtual void parseJson(const QString reply_json) = 0;

private:
    QNetworkAccessManager m_manager;

    QString m_service;
    QString m_did;
    QString m_handle;
    QString m_email;
    QString m_accessJwt;
    QString m_refreshJwt;
};

#endif // ACCESSATPROTOCOL_H
