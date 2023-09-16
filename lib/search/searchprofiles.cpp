#include "searchprofiles.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>

namespace SearchInterface {

SearchProfiles::SearchProfiles(QObject *parent) : AtProtocolInterface::AccessAtProtocol { parent }
{
}

void SearchProfiles::search(const QString &text)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("q"), text);

    get(QStringLiteral("search/profiles"), query, false);
}

const QStringList *SearchProfiles::didList() const
{
    return &m_didList;
}

bool SearchProfiles::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else if (json_doc.isArray()) {
        for (const auto &value : json_doc.array()) {
            QString did = value.toObject().value("did").toString();
            if (!did.isEmpty()) {
                m_didList.append(did);
            }
        }
    }

    return success;
}

}
