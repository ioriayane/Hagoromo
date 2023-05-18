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

void ComAtprotoServerRefreshSession::parseJson(const QString reply_json)
{
    bool success = false;

    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (!json_doc.isEmpty()) {
        setSession(json_doc.object().value("did").toString(),
                   json_doc.object().value("handle").toString(), email(),
                   json_doc.object().value("accessJwt").toString(),
                   json_doc.object().value("refreshJwt").toString());

        if (!did().isEmpty() && !handle().isEmpty() && !email().isEmpty() && !accessJwt().isEmpty()
            && !refreshJwt().isEmpty()) {
            success = true;
        }
    }

    emit finished(success);
}

}
