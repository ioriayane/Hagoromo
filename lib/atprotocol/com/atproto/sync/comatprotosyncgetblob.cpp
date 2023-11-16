#include "comatprotosyncgetblob.h"
#include "atprotocol/lexicons_func.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

ComAtprotoSyncGetBlob::ComAtprotoSyncGetBlob(QObject *parent) : AccessAtProtocol { parent } { }

void ComAtprotoSyncGetBlob::getBlob(const QString &did, const QString &cid)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("did"), did);
    query.addQueryItem(QStringLiteral("cid"), cid);

    get(QStringLiteral("xrpc/com.atproto.sync.getBlob"), query);
}

bool ComAtprotoSyncGetBlob::parseJson(bool success, const QString reply_json)
{
    Q_UNUSED(success)
    Q_UNUSED(reply_json)
    return false;
}

bool ComAtprotoSyncGetBlob::recvImage(const QByteArray &data)
{
    m_blobData = data;
    return true;
}

const QByteArray &ComAtprotoSyncGetBlob::blobData() const
{
    return m_blobData;
}

}
