#include "comatprotoserverrefreshsessionex.h"

namespace AtProtocolInterface {

ComAtprotoServerRefreshSessionEx::ComAtprotoServerRefreshSessionEx(QObject *parent)
    : ComAtprotoServerRefreshSession { parent }
{
}

void ComAtprotoServerRefreshSessionEx::refreshSession()
{
    setSession(account().did, account().handle, account().email, account().refreshJwt,
               account().refreshJwt);
    ComAtprotoServerRefreshSession::refreshSession();
}

const QString &ComAtprotoServerRefreshSessionEx::email() const
{
    if (ComAtprotoServerRefreshSession::email().isEmpty()) {
        return account().email;
    } else {
        // 取得出来ていたら使う
        return ComAtprotoServerRefreshSession::email();
    }
}

bool ComAtprotoServerRefreshSessionEx::parseJson(bool success, const QString reply_json)
{
    success = ComAtprotoServerRefreshSession::parseJson(success, reply_json);
    if (success) {
        setSession(did(), handle(), account().email, accessJwt(), refreshJwt());

        if (did().isEmpty() || handle().isEmpty() || accessJwt().isEmpty()
            || refreshJwt().isEmpty()) {
            success = false;
        }
    }
    return success;
}

}
