#ifndef COMATPROTOSERVERREFRESHSESSION_H
#define COMATPROTOSERVERREFRESHSESSION_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ComAtprotoServerRefreshSession : public AccessAtProtocol
{
public:
    explicit ComAtprotoServerRefreshSession(QObject *parent = nullptr);

    void refreshSession();

    const QString &accessJwt() const;
    const QString &refreshJwt() const;
    const QString &handle() const;
    const QString &did() const;
    const QVariant &didDoc() const;
    const QString &email() const;
    const bool &emailConfirmed() const;
    const bool &emailAuthFactor() const;
    const bool &active() const;
    const QString &status() const;

protected:
    virtual bool parseJson(bool success, const QString reply_json);

private:
    QString m_accessJwt;
    QString m_refreshJwt;
    QString m_handle;
    QString m_did;
    QVariant m_didDoc;
    QString m_email;
    bool m_emailConfirmed;
    bool m_emailAuthFactor;
    bool m_active;
    QString m_status;
};

}

#endif // COMATPROTOSERVERREFRESHSESSION_H
