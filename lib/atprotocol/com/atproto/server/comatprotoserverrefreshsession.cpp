#include "comatprotoserverrefreshsession.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

ComAtprotoServerRefreshSession::ComAtprotoServerRefreshSession(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ComAtprotoServerRefreshSession::refreshSession()
{
    setSession(did(), handle(), email(), refreshJwt(), refreshJwt());
    post(QStringLiteral("xrpc/com.atproto.server.refreshSession"), QByteArray(), true);
}

void ComAtprotoServerRefreshSession::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        setSession(json_doc.object().value("did").toString(),
                   json_doc.object().value("handle").toString(), email(),
                   json_doc.object().value("accessJwt").toString(),
                   json_doc.object().value("refreshJwt").toString());

        if (did().isEmpty() || handle().isEmpty() || email().isEmpty() || accessJwt().isEmpty()
            || refreshJwt().isEmpty()) {
            success = false;
        }
    }

    emit finished(success);
}

}
