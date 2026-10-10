#include "oauthrevoketoken.h"

namespace AtProtocolInterface {

OauthRevokeToken::OauthRevokeToken(QObject *parent) : AccessAtProtocol { parent } { }

void OauthRevokeToken::revokeToken(const QByteArray &payload)
{
    post(QString(""), payload, false);
}

bool OauthRevokeToken::parseJson(bool success, const QString reply_json)
{
    // 成功時の本文は空(または{})なので中身は見ない
    Q_UNUSED(reply_json)
    return success;
}

}
