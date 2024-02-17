#include "comatprotosyncgetblob.h"

#include <QUrlQuery>

namespace AtProtocolInterface {

ComAtprotoSyncGetBlob::ComAtprotoSyncGetBlob(QObject *parent) : AccessAtProtocol { parent } { }

void ComAtprotoSyncGetBlob::getBlob(const QString &did, const QString &cid)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("did"), did);
    query.addQueryItem(QStringLiteral("cid"), cid);

    get(QStringLiteral("xrpc/com.atproto.sync.getBlob"), query, false);
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
        m_extention = items.last();
    }

    return true;
}

QString ComAtprotoSyncGetBlob::extension() const
{
    return m_extention;
}

const QByteArray &ComAtprotoSyncGetBlob::blobData() const
{
    return m_blobData;
}

}
