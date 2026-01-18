#include "appbskycontactimportcontacts.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

AppBskyContactImportContacts::AppBskyContactImportContacts(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyContactImportContacts::importContacts(const QString &token,
                                                  const QList<QString> &contacts)
{
    QJsonObject json_obj;
    if (!token.isEmpty()) {
        json_obj.insert(QStringLiteral("token"), token);
    }
    for (const auto &value : contacts) {
        url_query.addQueryItem(QStringLiteral("contacts"), value);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/app.bsky.contact.importContacts"),
         json_doc.toJson(QJsonDocument::Compact));
}

const QList<AtProtocolType::AppBskyContactDefs::MatchAndContactIndex> &
AppBskyContactImportContacts::matchesAndContactIndexesList() const
{
    return m_matchesAndContactIndexesList;
}

bool AppBskyContactImportContacts::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains("matchesAndContactIndexes")) {
        success = false;
    } else {
        for (const auto &value : json_doc.object().value("matchesAndContactIndexes").toArray()) {
            AtProtocolType::AppBskyContactDefs::MatchAndContactIndex data;
            AtProtocolType::AppBskyContactDefs::copyMatchAndContactIndex(value.toObject(), data);
            m_matchesAndContactIndexesList.append(data);
        }
    }

    return success;
}

}
