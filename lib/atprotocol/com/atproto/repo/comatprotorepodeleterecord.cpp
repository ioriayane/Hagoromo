#include "comatprotorepodeleterecord.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

ComAtprotoRepoDeleteRecord::ComAtprotoRepoDeleteRecord(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ComAtprotoRepoDeleteRecord::deleteRecord(const QString &repo, const QString &collection, const QString &rkey, const QString &swapRecord, const QString &swapCommit)
{
    post(QStringLiteral("xrpc/com.atproto.repo.deleteRecord"), QByteArray(), true);
}

void ComAtprotoRepoDeleteRecord::parseJson(const QString reply_json)
{
    bool success = false;

    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
    }

    emit finished(success);
}

}