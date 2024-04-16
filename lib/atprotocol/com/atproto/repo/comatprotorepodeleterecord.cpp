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

bool ComAtprotoRepoDeleteRecord::parseJson(bool success, const QString reply_json)
{
    Q_UNUSED(reply_json)
    return success;
}

}
