#include "comatprotomoderationcreatereportex.h"

namespace AtProtocolInterface {

ComAtprotoModerationCreateReportEx::ComAtprotoModerationCreateReportEx(QObject *parent)
    : ComAtprotoModerationCreateReport { parent }
{
}

void ComAtprotoModerationCreateReportEx::reportPost(const QString &uri, const QString &cid,
                                                    const QString &text, const QString &reason)
{
    QJsonObject json_subject;
    json_subject.insert("$type", "com.atproto.repo.strongRef");
    json_subject.insert("cid", cid);
    json_subject.insert("uri", uri);

    createReport("com.atproto.moderation.defs#" + reason, text, json_subject);
}

void ComAtprotoModerationCreateReportEx::reportAccount(const QString &did, const QString &text,
                                                       const QString &reason)
{
    QJsonObject json_subject;
    json_subject.insert("$type", "com.atproto.admin.defs#repoRef");
    json_subject.insert("did", did);

    createReport("com.atproto.moderation.defs#" + reason, text, json_subject);
}

void ComAtprotoModerationCreateReportEx::reportMessage(const QString &did, const QString &convo_id,
                                                       const QString &message_id,
                                                       const QString &text, const QString &reason)
{
    QJsonObject json_subject;
    json_subject.insert("$type", "chat.bsky.convo.defs#messageRef");
    json_subject.insert("did", did);
    json_subject.insert("convoId", convo_id);
    json_subject.insert("messageId", message_id);

    createReport("com.atproto.moderation.defs#" + reason, text, json_subject);
}

bool ComAtprotoModerationCreateReportEx::parseJson(bool success, const QString reply_json)
{
    success = ComAtprotoModerationCreateReport::parseJson(success, reply_json);
    if (success) {
        qDebug().noquote() << "ComAtprotoModerationCreateReportEx";
        qDebug().noquote() << "  id:" << id();
        qDebug().noquote() << "  reasonType:" << reasonType();
        qDebug().noquote() << "  reason:" << reason();
        qDebug().noquote() << "  repoRef.did:" << repoRef().did;
        qDebug().noquote() << "  main.cid:" << main().cid;
        qDebug().noquote() << "  main.uri:" << main().uri;
        qDebug().noquote() << "  reportedBy:" << reportedBy();
        qDebug().noquote() << "  createdAt:" << createdAt();
    }
    return success;
}

}
