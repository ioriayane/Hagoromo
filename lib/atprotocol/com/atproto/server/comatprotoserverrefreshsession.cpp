#include "comatprotoserverrefreshsession.h"
#include "atprotocol/lexicons_func.h"

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

bool ComAtprotoServerRefreshSession::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::ComAtprotoServerDefs::DidDoc did_doc;
        AtProtocolType::ComAtprotoServerDefs::copyDidDoc(
                json_doc.object().value("didDoc").toObject(), did_doc);

        setSession(json_doc.object().value("did").toString(),
                   json_doc.object().value("handle").toString(), email(),
                   json_doc.object().value("accessJwt").toString(),
                   json_doc.object().value("refreshJwt").toString());
        if (!did_doc.service.isEmpty()) {
            setServiceEndpoint(did_doc.service.front().serviceEndpoint);
        }

        if (did().isEmpty() || handle().isEmpty() || accessJwt().isEmpty()
            || refreshJwt().isEmpty()) {
            success = false;
        }
    }

    return success;
}

}
