#include "comatprotoservergetserviceauth.h"
#include "atprotocol/lexicons_func.h"
#include "atprotocol/lexicons_func_unknown.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

ComAtprotoServerGetServiceAuth::ComAtprotoServerGetServiceAuth(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ComAtprotoServerGetServiceAuth::getServiceAuth(const QString &aud, const int exp,
                                                    const QString &lxm)
{
    QUrlQuery url_query;
    if (!aud.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("aud"), aud);
    }
    if (exp > 0) {
        url_query.addQueryItem(QStringLiteral("exp"), QString::number(exp));
    }
    if (!lxm.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("lxm"), lxm);
    }

    get(QStringLiteral("xrpc/com.atproto.server.getServiceAuth"), url_query);
}

const QString &ComAtprotoServerGetServiceAuth::token() const
{
    return m_token;
}

bool ComAtprotoServerGetServiceAuth::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("token"), m_token);
    }

    return success;
}

}
