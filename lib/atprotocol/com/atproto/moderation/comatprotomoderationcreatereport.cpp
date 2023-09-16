#include "comatprotomoderationcreatereport.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

ComAtprotoModerationCreateReport::ComAtprotoModerationCreateReport(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ComAtprotoModerationCreateReport::reportPost(const QString &uri, const QString &cid,
                                                  const QString &reason)
{
    QJsonObject json_subject;
    json_subject.insert("$type", "com.atproto.repo.strongRef");
    json_subject.insert("cid", cid);
    json_subject.insert("uri", uri);
    QJsonObject json_obj;
    json_obj.insert("subject", json_subject);
    json_obj.insert("reasonType", "com.atproto.moderation.defs#" + reason);
    QJsonDocument json_doc(json_obj);
    post(QStringLiteral("xrpc/com.atproto.moderation.createReport"),
         json_doc.toJson(QJsonDocument::Compact));
}

void ComAtprotoModerationCreateReport::reportAccount(const QString &did, const QString &reason)
{
    QJsonObject json_subject;
    json_subject.insert("$type", "com.atproto.admin.defs#repoRef");
    json_subject.insert("did", did);
    QJsonObject json_obj;
    json_obj.insert("subject", json_subject);
    json_obj.insert("reasonType", "com.atproto.moderation.defs#" + reason);
    QJsonDocument json_doc(json_obj);
    post(QStringLiteral("xrpc/com.atproto.moderation.createReport"),
         json_doc.toJson(QJsonDocument::Compact));
}

bool ComAtprotoModerationCreateReport::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) { }

    return success;
}

}
