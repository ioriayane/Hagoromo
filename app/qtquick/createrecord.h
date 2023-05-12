#ifndef CREATERECORD_H
#define CREATERECORD_H

#include "../atprotocol/lexicons.h"
#include "../atprotocol/accessatprotocol.h"
#include <QObject>

class CreateRecord : public QObject
{
    Q_OBJECT

public:
    explicit CreateRecord(QObject *parent = nullptr);

    Q_INVOKABLE void setAccount(const QString &service, const QString &did, const QString &handle,
                                const QString &email, const QString &accessJwt,
                                const QString &refreshJwt);
    Q_INVOKABLE void setReply(const QString &parent_cid, const QString &parent_uri,
                              const QString &root_cid, const QString &root_uri);
    Q_INVOKABLE void setQuote(const QString &cid, const QString &uri);

    Q_INVOKABLE void post(const QString &text);
    Q_INVOKABLE void repost(const QString &cid, const QString &uri);
    Q_INVOKABLE void like(const QString &cid, const QString &uri);

signals:
    void finished(bool success);

private:
    AtProtocolInterface::AccountData m_account;

    AtProtocolType::ComAtprotoRepoStrongRef::Main m_replyParent;
    AtProtocolType::ComAtprotoRepoStrongRef::Main m_replyRoot;
    AtProtocolType::ComAtprotoRepoStrongRef::Main m_embedQuote;
};

#endif // CREATERECORD_H
