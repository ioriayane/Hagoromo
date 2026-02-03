#include "appbskycontactstartphoneverification.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

AppBskyContactStartPhoneVerification::AppBskyContactStartPhoneVerification(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyContactStartPhoneVerification::startPhoneVerification(const QString &phone)
{
    QJsonObject json_obj;
    if (!phone.isEmpty()) {
        json_obj.insert(QStringLiteral("phone"), phone);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/app.bsky.contact.startPhoneVerification"),
         json_doc.toJson(QJsonDocument::Compact));
}

bool AppBskyContactStartPhoneVerification::parseJson(bool success, const QString reply_json)
{
    Q_UNUSED(reply_json)
    return success;
}

}
