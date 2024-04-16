#include "comatprotomoderationcreatereport.h"
#include "atprotocol/lexicons_func.h"
#include "atprotocol/lexicons_func_unknown.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

ComAtprotoModerationCreateReport::ComAtprotoModerationCreateReport(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ComAtprotoModerationCreateReport::createReport(const QString &reasonType,
                                                    const QString &reason,
                                                    const QJsonObject &subject)
{
    QJsonObject json_obj;
    if (!reasonType.isEmpty()) {
        json_obj.insert(QStringLiteral("reasonType"), reasonType);
    }
    if (!reason.isEmpty()) {
        json_obj.insert(QStringLiteral("reason"), reason);
    }
    if (!subject.isEmpty()) {
        json_obj.insert(QStringLiteral("subject"), subject);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/com.atproto.moderation.createReport"),
         json_doc.toJson(QJsonDocument::Compact));
}

const int &ComAtprotoModerationCreateReport::id() const
{
    return m_id;
}

const AtProtocolType::ComAtprotoModerationDefs::ReasonType &
ComAtprotoModerationCreateReport::reasonType() const
{
    return m_reasonType;
}

const QString &ComAtprotoModerationCreateReport::reason() const
{
    return m_reason;
}

const AtProtocolType::ComAtprotoAdminDefs::RepoRef &
ComAtprotoModerationCreateReport::repoRef() const
{
    return m_repoRef;
}

const AtProtocolType::ComAtprotoRepoStrongRef::Main &ComAtprotoModerationCreateReport::main() const
{
    return m_main;
}

const QString &ComAtprotoModerationCreateReport::reportedBy() const
{
    return m_reportedBy;
}

const QString &ComAtprotoModerationCreateReport::createdAt() const
{
    return m_createdAt;
}

bool ComAtprotoModerationCreateReport::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::LexiconsTypeUnknown::copyInt(json_doc.object().value("id"), m_id);
        AtProtocolType::ComAtprotoModerationDefs::copyReasonType(
                json_doc.object().value("reasonType").toObject(), m_reasonType);
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("reason"),
                                                        m_reason);
        QString type;
        type = json_doc.object().value("subject").toObject().value("$type").toString();
        if (type == QStringLiteral("com.atproto.admin.defs#repoRef")) {
            AtProtocolType::ComAtprotoAdminDefs::copyRepoRef(
                    json_doc.object().value("subject").toObject(), m_repoRef);
        } else if (type == QStringLiteral("com.atproto.repo.strongRef")) {
            AtProtocolType::ComAtprotoRepoStrongRef::copyMain(
                    json_doc.object().value("subject").toObject(), m_main);
        }
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("reportedBy"),
                                                        m_reportedBy);
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("createdAt"),
                                                        m_createdAt);
    }

    return success;
}

}
