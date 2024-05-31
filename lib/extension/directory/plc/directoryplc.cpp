#include "directoryplc.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

DirectoryPlc::DirectoryPlc(QObject *parent) : AccessAtProtocol { parent }
{
    m_defaultService = "https://plc.directory";
}

void DirectoryPlc::directory(const QString &did)
{
    QUrlQuery url_query;

    if (service().isEmpty()) {
        setService(defaultService());
    }

    get(did, url_query, false);
}

const AtProtocolType::DirectoryPlcDefs::DidDoc &DirectoryPlc::didDoc() const
{
    return m_didDoc;
}

QString DirectoryPlc::serviceEndpoint() const
{
    for (const auto &service : m_didDoc.service) {
        if (service.id == "#atproto_pds" && service.type == "AtprotoPersonalDataServer") {
            return service.serviceEndpoint;
        }
    }
    return QString();
}

bool DirectoryPlc::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::DirectoryPlcDefs::copyDidDoc(json_doc.object(), m_didDoc);
    }

    return success;
}

QString DirectoryPlc::defaultService() const
{
    return m_defaultService;
}
}
