#ifndef COMATPROTOMODERATIONCREATEREPORT_H
#define COMATPROTOMODERATIONCREATEREPORT_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class ComAtprotoModerationCreateReport : public AccessAtProtocol
{
public:
    explicit ComAtprotoModerationCreateReport(QObject *parent = nullptr);

    void createReport(const QString &reasonType, const QString &reason, const QJsonObject &subject);

    const qint64 &id() const;
    const AtProtocolType::ComAtprotoModerationDefs::ReasonType &reasonType() const;
    const QString &reason() const;
    const AtProtocolType::ComAtprotoAdminDefs::RepoRef &repoRef() const;
    const AtProtocolType::ComAtprotoRepoStrongRef::Main &main() const;
    const QString &reportedBy() const;
    const QString &createdAt() const;

protected:
    virtual bool parseJson(bool success, const QString reply_json);

private:
    qint64 m_id;
    AtProtocolType::ComAtprotoModerationDefs::ReasonType m_reasonType;
    QString m_reason;
    AtProtocolType::ComAtprotoAdminDefs::RepoRef m_repoRef;
    AtProtocolType::ComAtprotoRepoStrongRef::Main m_main;
    QString m_reportedBy;
    QString m_createdAt;
};

}

#endif // COMATPROTOMODERATIONCREATEREPORT_H
