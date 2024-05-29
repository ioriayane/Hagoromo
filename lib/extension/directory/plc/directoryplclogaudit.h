#ifndef DIRECTORYPLCLOGAUDIT_H
#define DIRECTORYPLCLOGAUDIT_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class DirectoryPlcLogAudit : public AccessAtProtocol
{
public:
    explicit DirectoryPlcLogAudit(QObject *parent = nullptr);

    void audit(const QString &did);

    const AtProtocolType::DirectoryPlcDefs::PlcAuditLog &plcAuditLog() const;

    QString defaultService() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::DirectoryPlcDefs::PlcAuditLog m_plcAuditLog;
    QString m_defaultService;
};

}

#endif // DIRECTORYPLCLOGAUDIT_H
