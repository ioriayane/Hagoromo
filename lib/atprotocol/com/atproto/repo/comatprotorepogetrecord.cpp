#include "comatprotorepogetrecord.h"
#include "atprotocol/lexicons_func_unknown.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

using namespace AtProtocolType;

namespace AtProtocolInterface {

ComAtprotoRepoGetRecord::ComAtprotoRepoGetRecord(QObject *parent) : AccessAtProtocol { parent } { }

void ComAtprotoRepoGetRecord::getRecord(const QString &repo, const QString &collection,
                                        const QString &rkey, const QString &cid)
{
    QUrlQuery query;
    if (!repo.isEmpty()) {
        query.addQueryItem(QStringLiteral("repo"), repo);
    }
    if (!collection.isEmpty()) {
        query.addQueryItem(QStringLiteral("collection"), collection);
    }
    if (!rkey.isEmpty()) {
        query.addQueryItem(QStringLiteral("rkey"), rkey);
    }
    if (!cid.isEmpty()) {
        query.addQueryItem(QStringLiteral("cid"), cid);
    }

    get(QStringLiteral("xrpc/com.atproto.repo.getRecord"), query);
}

void ComAtprotoRepoGetRecord::profile(const QString &did)
{
    getRecord(did, QStringLiteral("app.bsky.actor.profile"), QStringLiteral("self"), QString());
}

void ComAtprotoRepoGetRecord::list(const QString &did, const QString &rkey)
{
    getRecord(did, QStringLiteral("app.bsky.graph.list"), rkey, QString());
}

bool ComAtprotoRepoGetRecord::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        m_cid = json_doc.object().value("cid").toString();
        m_uri = json_doc.object().value("uri").toString();
        LexiconsTypeUnknown::copyUnknown(json_doc.object().value("value").toObject(), m_record);
    }

    return success;
}

QString ComAtprotoRepoGetRecord::cid() const
{
    return m_cid;
}

QString ComAtprotoRepoGetRecord::uri() const
{
    return m_uri;
}

QVariant ComAtprotoRepoGetRecord::record() const
{
    return m_record;
}

}
