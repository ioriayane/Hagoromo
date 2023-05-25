#include "comatprotorepodeleterecord.h"

#include <QJsonDocument>
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
    json_obj.insert("repo", repo);
    json_obj.insert("collection", collection);
    json_obj.insert("rkey", rkey);
    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/com.atproto.repo.deleteRecord"),
         json_doc.toJson(QJsonDocument::Compact));
}

void ComAtprotoRepoDeleteRecord::unfollow(const QString &rkey)
{
    deleteRecord(this->did(), QStringLiteral("app.bsky.graph.follow"), rkey, QString(), QString());
}

void ComAtprotoRepoDeleteRecord::parseJson(const QString reply_json)
{
    //    bool success = false;

    //    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    //    if (json_doc.isEmpty()) {
    //    } else {
    //        success = true;
    //    }

    emit finished(true);
}

}
