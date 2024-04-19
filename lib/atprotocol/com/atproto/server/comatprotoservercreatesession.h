#ifndef COMATPROTOSERVERCREATESESSION_H
#define COMATPROTOSERVERCREATESESSION_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ComAtprotoServerCreateSession : public AccessAtProtocol
{
public:
    explicit ComAtprotoServerCreateSession(QObject *parent = nullptr);

    void createSession(const QString &identifier, const QString &password);

    const QString &accessJwt() const;
    const QString &refreshJwt() const;
    const QString &handle() const;
    const QString &did() const;
    const QVariant &didDoc() const;
    const QString &email() const;
    const bool &emailConfirmed() const;

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
};

}

#endif // COMATPROTOSERVERCREATESESSION_H
