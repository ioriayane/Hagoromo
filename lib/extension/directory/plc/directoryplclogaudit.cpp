#include "directoryplclogaudit.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

DirectoryPlcLogAudit::DirectoryPlcLogAudit(QObject *parent) : AccessAtProtocol { parent } { }

void DirectoryPlcLogAudit::audit(const QString &did)
{
    QUrlQuery url_query;

    setService("https://plc.directory");

    get(did + "/log/audit", url_query, false);
}

const AtProtocolType::DirectoryPlcDefs::PlcAuditLog &DirectoryPlcLogAudit::plcAuditLog() const
{
    return m_plcAuditLog;
}

bool DirectoryPlcLogAudit::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::DirectoryPlcDefs::copyPlcAuditLog(json_doc.array(), m_plcAuditLog);
    }

    return success;
}

}
