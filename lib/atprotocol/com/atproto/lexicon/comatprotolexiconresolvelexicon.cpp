#include "comatprotolexiconresolvelexicon.h"
#include "atprotocol/lexicons_func.h"
#include "atprotocol/lexicons_func_unknown.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

ComAtprotoLexiconResolveLexicon::ComAtprotoLexiconResolveLexicon(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ComAtprotoLexiconResolveLexicon::resolveLexicon(const QString &nsid)
{
    QUrlQuery url_query;
    if (!nsid.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("nsid"), nsid);
    }

    get(QStringLiteral("xrpc/com.atproto.lexicon.resolveLexicon"), url_query);
}

const QString &ComAtprotoLexiconResolveLexicon::cid() const
{
    return m_cid;
}

const AtProtocolType::ComAtprotoLexiconSchema::Main &ComAtprotoLexiconResolveLexicon::schema() const
{
    return m_schema;
}

const QString &ComAtprotoLexiconResolveLexicon::uri() const
{
    return m_uri;
}

bool ComAtprotoLexiconResolveLexicon::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("cid"), m_cid);
        AtProtocolType::ComAtprotoLexiconSchema::copyMain(
                json_doc.object().value("schema").toObject(), m_schema);
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("uri"), m_uri);
    }

    return success;
}

}
