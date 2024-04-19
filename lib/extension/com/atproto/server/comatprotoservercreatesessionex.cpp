#include "comatprotoservercreatesessionex.h"

namespace AtProtocolInterface {

ComAtprotoServerCreateSessionEx::ComAtprotoServerCreateSessionEx(QObject *parent)
    : ComAtprotoServerCreateSession { parent }
{
}

bool ComAtprotoServerCreateSessionEx::parseJson(bool success, const QString reply_json)
{
    success = ComAtprotoServerCreateSession::parseJson(success, reply_json);

    setSession(did(), handle(), email(), accessJwt(), refreshJwt());

    if (did().isEmpty() || handle().isEmpty() || email().isEmpty() || accessJwt().isEmpty()
        || refreshJwt().isEmpty()) {
        success = false;
    }

    return success;
}

}
