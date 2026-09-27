#include "wellknownatprotodid.h"

#include <QUrlQuery>

namespace AtProtocolInterface {

WellKnownAtprotoDid::WellKnownAtprotoDid(QObject *parent) : AccessAtProtocol { parent } { }

void WellKnownAtprotoDid::atprotoDid()
{
    QUrlQuery url_query;

    get(QStringLiteral(".well-known/atproto-did"), url_query, false);
}

QString WellKnownAtprotoDid::did() const
{
    return m_did;
}

bool WellKnownAtprotoDid::parseJson(bool success, const QString reply_json)
{
    Q_UNUSED(success)
    Q_UNUSED(reply_json)
    // text/plainで返るのでJSONは受け付けない
    return false;
}

bool WellKnownAtprotoDid::recvImage(const QByteArray &data, const QString &content_type)
{
    Q_UNUSED(content_type)
    m_did = QString::fromUtf8(data).trimmed();
    return m_did.startsWith("did:");
}

bool WellKnownAtprotoDid::isRawContentType(const QString &content_type) const
{
    return content_type.startsWith("text/plain");
}

}
