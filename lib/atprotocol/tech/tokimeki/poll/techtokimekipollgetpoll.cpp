#include "techtokimekipollgetpoll.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

TechTokimekiPollGetPoll::TechTokimekiPollGetPoll(QObject *parent) : AccessAtProtocol { parent } { }

void TechTokimekiPollGetPoll::getPoll(const QString &uri, const QString &viewer)
{
    QUrlQuery url_query;
    if (!uri.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("uri"), uri);
    }
    if (!viewer.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("viewer"), viewer);
    }

    get(QStringLiteral("xrpc/tech.tokimeki.poll.getPoll"), url_query, false);
}

const AtProtocolType::TechTokimekiPollDefs::PollViewDetailed &
TechTokimekiPollGetPoll::pollViewDetailed() const
{
    return m_pollViewDetailed;
}

bool TechTokimekiPollGetPoll::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::TechTokimekiPollDefs::copyPollViewDetailed(json_doc.object(),
                                                                   m_pollViewDetailed);
    }

    return success;
}

}
