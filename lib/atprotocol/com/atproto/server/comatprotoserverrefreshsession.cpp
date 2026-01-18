#include "comatprotoserverrefreshsession.h"
#include "atprotocol/lexicons_func.h"
#include "atprotocol/lexicons_func_unknown.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

ComAtprotoServerRefreshSession::ComAtprotoServerRefreshSession(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ComAtprotoServerRefreshSession::refreshSession()
{
    post(QStringLiteral("xrpc/com.atproto.server.refreshSession"), QByteArray());
}

const QString &ComAtprotoServerRefreshSession::accessJwt() const
{
    return m_accessJwt;
}

const QString &ComAtprotoServerRefreshSession::refreshJwt() const
{
    return m_refreshJwt;
}

const QString &ComAtprotoServerRefreshSession::handle() const
{
    return m_handle;
}

const QString &ComAtprotoServerRefreshSession::did() const
{
    return m_did;
}

const QVariant &ComAtprotoServerRefreshSession::didDoc() const
{
    return m_didDoc;
}

const QString &ComAtprotoServerRefreshSession::email() const
{
    return m_email;
}

const bool &ComAtprotoServerRefreshSession::emailConfirmed() const
{
    return m_emailConfirmed;
}

const bool &ComAtprotoServerRefreshSession::emailAuthFactor() const
{
    return m_emailAuthFactor;
}

const bool &ComAtprotoServerRefreshSession::active() const
{
    return m_active;
}

const QString &ComAtprotoServerRefreshSession::status() const
{
    return m_status;
}

bool ComAtprotoServerRefreshSession::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("accessJwt"),
                                                        m_accessJwt);
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("refreshJwt"),
                                                        m_refreshJwt);
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("handle"),
                                                        m_handle);
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("did"), m_did);
        AtProtocolType::LexiconsTypeUnknown::copyUnknown(
                json_doc.object().value("didDoc").toObject(), m_didDoc);
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("email"), m_email);
        AtProtocolType::LexiconsTypeUnknown::copyBool(json_doc.object().value("emailConfirmed"),
                                                      m_emailConfirmed);
        AtProtocolType::LexiconsTypeUnknown::copyBool(json_doc.object().value("emailAuthFactor"),
                                                      m_emailAuthFactor);
        AtProtocolType::LexiconsTypeUnknown::copyBool(json_doc.object().value("active"), m_active);
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("status"),
                                                        m_status);
    }

    return success;
}

}
