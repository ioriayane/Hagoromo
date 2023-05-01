#ifndef ACCESSATPROTOCOL_H
#define ACCESSATPROTOCOL_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>

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
    QString handle() const;
    QString email() const;
    QString accessJwt() const;
    QString refreshJwt() const;

signals:
    void finished(bool success);

public slots:

protected:
    void get(const QString &endpoint);
    void post(const QString &endpoint, const QByteArray &json);

    void setDid(const QString &newDid);
    void setHandle(const QString &newHandle);
    void setEmail(const QString &newEmail);
    void setAccessJwt(const QString &newAccessJwt);
    void setRefreshJwt(const QString &newRefreshJwt);

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
