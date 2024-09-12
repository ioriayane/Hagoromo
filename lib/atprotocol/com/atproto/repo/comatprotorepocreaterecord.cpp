#include "comatprotorepocreaterecord.h"
#include "atprotocol/lexicons_func.h"
#include "atprotocol/lexicons_func_unknown.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

ComAtprotoRepoCreateRecord::ComAtprotoRepoCreateRecord(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ComAtprotoRepoCreateRecord::createRecord(const QString &repo, const QString &collection,
                                              const QString &rkey, const bool validate,
                                              const QJsonObject &record, const QString &swapCommit)
{
    QJsonObject json_obj;
    if (!repo.isEmpty()) {
        json_obj.insert(QStringLiteral("repo"), repo);
    }
    if (!collection.isEmpty()) {
        json_obj.insert(QStringLiteral("collection"), collection);
    }
    if (!rkey.isEmpty()) {
        json_obj.insert(QStringLiteral("rkey"), rkey);
    }
    json_obj.insert(QStringLiteral("validate"), validate);
    if (!record.isEmpty()) {
        json_obj.insert(QStringLiteral("record"), record);
    }
    if (!swapCommit.isEmpty()) {
        json_obj.insert(QStringLiteral("swapCommit"), swapCommit);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/com.atproto.repo.createRecord"),
         json_doc.toJson(QJsonDocument::Compact));
}

const QString &ComAtprotoRepoCreateRecord::uri() const
{
    return m_uri;
}

const QString &ComAtprotoRepoCreateRecord::cid() const
{
    return m_cid;
}

const AtProtocolType::ComAtprotoRepoDefs::CommitMeta &ComAtprotoRepoCreateRecord::commit() const
{
    return m_commit;
}

const QString &ComAtprotoRepoCreateRecord::validationStatus() const
{
    return m_validationStatus;
}

bool ComAtprotoRepoCreateRecord::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("uri"), m_uri);
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("cid"), m_cid);
        AtProtocolType::ComAtprotoRepoDefs::copyCommitMeta(
                json_doc.object().value("commit").toObject(), m_commit);
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("validationStatus"),
                                                        m_validationStatus);
    }

    return success;
}

}
