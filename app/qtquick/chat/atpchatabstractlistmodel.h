#ifndef ATPCHATABSTRACTLISTMODEL_H
#define ATPCHATABSTRACTLISTMODEL_H

#include "atprotocol/accessatprotocol.h"

#include <QAbstractListModel>

class AtpChatAbstractListModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)

public:
    explicit AtpChatAbstractListModel(QObject *parent = nullptr);

    Q_INVOKABLE void clear();

    AtProtocolInterface::AccountData account() const;
    Q_INVOKABLE void setAccount(const QString &service, const QString &did, const QString &handle,
                                const QString &email, const QString &accessJwt,
                                const QString &refreshJwt);
    void setServiceEndpoint(const QString &service_endpoint);

    virtual Q_INVOKABLE bool getLatest() = 0;
    virtual Q_INVOKABLE bool getNext() = 0;

    bool running() const;
    void setRunning(bool newRunning);

signals:
    void errorOccured(const QString &code, const QString &message);
    void runningChanged();

protected:
    void getServiceEndpoint(std::function<void()> callback);

    QString m_cursor;

    const QString headerName = "atproto-proxy";
    const QString headerValue = "did:web:api.bsky.chat#bsky_chat";

private:
    AtProtocolInterface::AccountData m_account;
    bool m_running;
};

#endif // ATPCHATABSTRACTLISTMODEL_H
