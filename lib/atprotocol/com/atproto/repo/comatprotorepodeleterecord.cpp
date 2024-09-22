#include "comatprotorepodeleterecord.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

ComAtprotoRepoDeleteRecord::ComAtprotoRepoDeleteRecord(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ComAtprotoRepoDeleteRecord::deleteRecord(const QString &repo, const QString &collection,
                                              const QString &rkey, const QString &swapRecord,
                                              const QString &swapCommit)
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
    if (!swapRecord.isEmpty()) {
        json_obj.insert(QStringLiteral("swapRecord"), swapRecord);
    }
    if (!swapCommit.isEmpty()) {
        json_obj.insert(QStringLiteral("swapCommit"), swapCommit);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/com.atproto.repo.deleteRecord"),
         json_doc.toJson(QJsonDocument::Compact));
}

const AtProtocolType::ComAtprotoRepoDefs::CommitMeta &ComAtprotoRepoDeleteRecord::commit() const
{
    return m_commit;
}

bool ComAtprotoRepoDeleteRecord::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::ComAtprotoRepoDefs::copyCommitMeta(
                json_doc.object().value("commit").toObject(), m_commit);
    }

    return success;
}

}
