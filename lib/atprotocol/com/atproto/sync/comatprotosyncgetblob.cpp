#include "comatprotosyncgetblob.h"
#include "atprotocol/lexicons_func.h"
#include "atprotocol/lexicons_func_unknown.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

ComAtprotoSyncGetBlob::ComAtprotoSyncGetBlob(QObject *parent) : AccessAtProtocol { parent } { }

void ComAtprotoSyncGetBlob::getBlob(const QString &did, const QString &cid)
{
    QUrlQuery url_query;
    if (!did.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("did"), did);
    }
    if (!cid.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("cid"), cid);
    }

    get(QStringLiteral("xrpc/com.atproto.sync.getBlob"), url_query, false);
}

const QByteArray &ComAtprotoSyncGetBlob::blobData() const
{
    return m_blobData;
}

const QString &ComAtprotoSyncGetBlob::extension() const
{
    return m_extension;
}

bool ComAtprotoSyncGetBlob::parseJson(bool success, const QString reply_json)
{
    Q_UNUSED(success)
    Q_UNUSED(reply_json)
    return false;
}

bool ComAtprotoSyncGetBlob::recvImage(const QByteArray &data, const QString &content_type)
{
    m_blobData = data;

    QStringList items = content_type.split("/");
    if (items.length() == 2) {
        m_extension = items.last();
    }

    return true;
}

}
