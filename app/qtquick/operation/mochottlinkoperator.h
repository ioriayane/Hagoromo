#ifndef MOCHOTTLINKOPERATOR_H
#define MOCHOTTLINKOPERATOR_H

#include "atprotocol/accessatprotocol.h"

#include <QObject>
#include <QString>

// mochottの長文投稿の前に、アカウントの連携とpublicationを確認する
class MochottLinkOperator : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(bool authorized READ authorized NOTIFY stateChanged)
    Q_PROPERTY(bool ready READ ready NOTIFY stateChanged)
    Q_PROPERTY(QString authorizeUrl READ authorizeUrl NOTIFY stateChanged)
    Q_PROPERTY(QString profileUri READ profileUri NOTIFY stateChanged)
    Q_PROPERTY(QString standardPublication READ standardPublication NOTIFY stateChanged)

public:
    explicit MochottLinkOperator(QObject *parent = nullptr);

    // アカウントが変わったときは確認結果を消す
    Q_INVOKABLE void setAccount(const QString &uuid);
    Q_INVOKABLE void setServiceUrl(const QString &url);
    // OAuthのときはmochottへの書き込みが許可されているか
    Q_INVOKABLE bool hasRequiredScope() const;
    Q_INVOKABLE void check();
    Q_INVOKABLE void reset();

    bool running() const;
    bool authorized() const;
    bool ready() const;
    QString authorizeUrl() const;
    QString profileUri() const;
    QString standardPublication() const;

signals:
    void finished(bool success);
    void errorOccurred(const QString &code, const QString &message);
    void runningChanged();
    void stateChanged();

private:
    AtProtocolInterface::AccountData account() const;
    void setRunning(bool running);
    void getPublication();
    void finish(bool success, const QString &code = QString(), const QString &message = QString());

    QString m_uuid;
    QString m_serviceUrl;
    bool m_running;
    bool m_authorized;
    QString m_authorizeUrl;
    QString m_profileUri;
    QString m_standardPublication;
};

#endif // MOCHOTTLINKOPERATOR_H
