#include "appbskycontactdismissmatch.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

AppBskyContactDismissMatch::AppBskyContactDismissMatch(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyContactDismissMatch::dismissMatch(const QString &subject)
{
    QJsonObject json_obj;
    if (!subject.isEmpty()) {
        json_obj.insert(QStringLiteral("subject"), subject);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/app.bsky.contact.dismissMatch"),
         json_doc.toJson(QJsonDocument::Compact));
}

bool AppBskyContactDismissMatch::parseJson(bool success, const QString reply_json)
{
    Q_UNUSED(reply_json)
    return success;
}

}
