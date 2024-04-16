#include "comatprotorepogetrecord.h"
#include "atprotocol/lexicons_func.h"
#include "atprotocol/lexicons_func_unknown.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

ComAtprotoRepoGetRecord::ComAtprotoRepoGetRecord(QObject *parent) : AccessAtProtocol { parent } { }

void ComAtprotoRepoGetRecord::getRecord(const QString &repo, const QString &collection,
                                        const QString &rkey, const QString &cid)
{
    QUrlQuery url_query;
    if (!repo.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("repo"), repo);
    }
    if (!collection.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("collection"), collection);
    }
    if (!rkey.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("rkey"), rkey);
    }
    if (!cid.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("cid"), cid);
    }

    get(QStringLiteral("xrpc/com.atproto.repo.getRecord"), url_query);
}

const QString &ComAtprotoRepoGetRecord::uri() const
{
    return m_uri;
}

const QString &ComAtprotoRepoGetRecord::cid() const
{
    return m_cid;
}

const QVariant &ComAtprotoRepoGetRecord::value() const
{
    return m_value;
}

bool ComAtprotoRepoGetRecord::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("uri"), m_uri);
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("cid"), m_cid);
        AtProtocolType::LexiconsTypeUnknown::copyUnknown(
                json_doc.object().value("value").toObject(), m_value);
    }

    return success;
}

}
