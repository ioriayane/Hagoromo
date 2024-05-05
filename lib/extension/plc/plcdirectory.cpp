#include "plcdirectory.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

PlcDirectory::PlcDirectory(QObject *parent) : AccessAtProtocol { parent } { }

void PlcDirectory::directory(const QString &did)
{
    QUrlQuery url_query;

    setService("https://plc.directory");

    get(did, url_query, false);
}

const AtProtocolType::ComAtprotoServerDefs::DidDoc &PlcDirectory::didDoc() const
{
    return m_didDoc;
}

QString PlcDirectory::serviceEndpoint() const
{
    for (const auto &service : m_didDoc.service) {
        if (service.id == "#atproto_pds" && service.type == "AtprotoPersonalDataServer") {
            return service.serviceEndpoint;
        }
    }
    return QString();
}

bool PlcDirectory::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::ComAtprotoServerDefs::copyDidDoc(json_doc.object(), m_didDoc);
    }

    return success;
}

}
