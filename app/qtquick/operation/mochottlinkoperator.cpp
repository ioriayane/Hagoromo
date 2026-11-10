#include "mochottlinkoperator.h"
#include "account/accountmanager.h"
#include "extension/site/mochott/sitemochottaccountgetauthorization.h"
#include "extension/site/mochott/sitemochottpublicationgetpublication.h"

using AtProtocolInterface::SiteMochottAccountGetAuthorization;
using AtProtocolInterface::SiteMochottPublicationGetPublication;

namespace {
const QString MOCHOTT_URL = QStringLiteral("https://mochott.site");

// 書き込むアカウントのレコードか
bool isOwnRecord(const QString &uri, const QString &did)
{
    return uri.startsWith(QString("at://%1/").arg(did));
}
}

MochottLinkOperator::MochottLinkOperator(QObject *parent)
    : QObject { parent }, m_serviceUrl(MOCHOTT_URL), m_running(false), m_authorized(false)
{
}

void MochottLinkOperator::setAccount(const QString &uuid)
{
    if (m_uuid == uuid)
        return;
    m_uuid = uuid;
    reset();
}

void MochottLinkOperator::setServiceUrl(const QString &url)
{
    if (url.isEmpty())
        return;
    m_serviceUrl = url;
}

bool MochottLinkOperator::hasRequiredScope() const
{
    const auto current = account();
    if (current.auth_type != AtProtocolInterface::AuthType::OAuth)
        return true;
    return current.scope.contains(AtProtocolInterface::AccountScope::Mochott);
}

void MochottLinkOperator::check()
{
    if (running() || m_uuid.isEmpty())
        return;
    setRunning(true);

    auto get_authorization = new SiteMochottAccountGetAuthorization(this);
    connect(get_authorization, &SiteMochottAccountGetAuthorization::finished, this,
            [=](bool success) {
                if (!success) {
                    finish(false, get_authorization->errorCode(),
                           get_authorization->errorMessage());
                } else {
                    m_authorized = get_authorization->authorized();
                    // mochott以外のページは開かない
                    const QString url = get_authorization->authorizeUrl();
                    m_authorizeUrl = url.startsWith(MOCHOTT_URL + "/") ? url : QString();
                    emit stateChanged();
                    if (m_authorized) {
                        getPublication();
                    } else {
                        finish(true);
                    }
                }
                get_authorization->deleteLater();
            });
    get_authorization->setAccount(account());
    get_authorization->getAuthorization();
}

void MochottLinkOperator::reset()
{
    m_authorized = false;
    m_authorizeUrl.clear();
    m_profileUri.clear();
    m_standardPublication.clear();
    emit stateChanged();
}

void MochottLinkOperator::getPublication()
{
    const QString did = account().did;
    auto get_publication = new SiteMochottPublicationGetPublication(this);
    connect(get_publication, &SiteMochottPublicationGetPublication::finished, this,
            [=](bool success) {
                if (!success) {
                    finish(false, get_publication->errorCode(), get_publication->errorMessage());
                } else if (!isOwnRecord(get_publication->uri(), did)) {
                    finish(false, QStringLiteral("InvalidPublication"),
                           QStringLiteral("The mochott publication belongs to another account."));
                } else {
                    // 記事のprofileは形を変えずにそのまま使う
                    m_profileUri = get_publication->uri();
                    m_standardPublication = isOwnRecord(get_publication->standardPublication(), did)
                            ? get_publication->standardPublication()
                            : QString();
                    emit stateChanged();
                    finish(true);
                }
                get_publication->deleteLater();
            });
    get_publication->setService(m_serviceUrl);
    get_publication->getPublication(did);
}

void MochottLinkOperator::finish(bool success, const QString &code, const QString &message)
{
    if (!success) {
        emit errorOccurred(code, message);
    }
    setRunning(false);
    emit finished(success);
}

bool MochottLinkOperator::running() const
{
    return m_running;
}

void MochottLinkOperator::setRunning(bool running)
{
    if (m_running == running)
        return;
    m_running = running;
    emit runningChanged();
}

bool MochottLinkOperator::authorized() const
{
    return m_authorized;
}

bool MochottLinkOperator::ready() const
{
    return m_authorized && !m_profileUri.isEmpty();
}

QString MochottLinkOperator::authorizeUrl() const
{
    return m_authorizeUrl;
}

QString MochottLinkOperator::profileUri() const
{
    return m_profileUri;
}

QString MochottLinkOperator::standardPublication() const
{
    return m_standardPublication;
}

AtProtocolInterface::AccountData MochottLinkOperator::account() const
{
    return AccountManager::getInstance()->getAccount(m_uuid);
}
