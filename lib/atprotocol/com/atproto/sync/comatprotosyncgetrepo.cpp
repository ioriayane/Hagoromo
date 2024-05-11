#include "comatprotosyncgetrepo.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

ComAtprotoSyncGetRepo::ComAtprotoSyncGetRepo(QObject *parent) : AccessAtProtocol { parent } { }

void ComAtprotoSyncGetRepo::getRepo(const QString &did, const QString &since)
{
    QUrlQuery url_query;
    if (!did.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("did"), did);
    }
    if (!since.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("since"), since);
    }

    get(QStringLiteral("xrpc/com.atproto.sync.getRepo"), url_query, false);
}

const QByteArray &ComAtprotoSyncGetRepo::repo() const
{
    return m_repo;
}

bool ComAtprotoSyncGetRepo::parseJson(bool success, const QString reply_json)
{
    Q_UNUSED(reply_json)
    return success;
}

bool ComAtprotoSyncGetRepo::recvImage(const QByteArray &data, const QString &content_type)
{
    Q_UNUSED(content_type)

    m_repo = data;

    return true;
}
}
