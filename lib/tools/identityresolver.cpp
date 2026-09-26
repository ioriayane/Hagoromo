#include "identityresolver.h"
#include "atprotocol/lexicons.h"
#include "extension/com/atproto/identity/comatprotoidentityresolvehandle.h"
#include "extension/directory/plc/directoryplc.h"
#include "extension/well-known/wellknownatprotodid.h"

#include <QDebug>
#include <QDnsLookup>
#include <QRegularExpression>
#include <QTimer>
#include <QUrl>

using AtProtocolInterface::ComAtprotoIdentityResolveHandle;
using AtProtocolInterface::DirectoryPlc;
using AtProtocolInterface::WellKnownAtprotoDid;

IdentityResolver::IdentityResolver(QObject *parent)
    : QObject { parent },
      m_plcDirectory(QStringLiteral("https://plc.directory")),
      m_dnsTimeout(5000)
{
}

void IdentityResolver::resolve(const QString &identifier)
{
    m_did.clear();
    m_handle.clear();
    m_pdsEndpoint.clear();

    QString id = identifier.trimmed();
    if (id.startsWith("@")) {
        id = id.mid(1);
    }
    if (id.startsWith("did:")) {
        if (!isValidDid(id)) {
            finish(false, "Invalid identifier", QString("'%1' is an invalid DID.").arg(id));
            return;
        }
        resolveFromDid(id);
    } else {
        const QString handle = normalizeHandle(id);
        if (!isValidHandle(handle)) {
            finish(false, "Invalid identifier", QString("'%1' is an invalid handle.").arg(id));
            return;
        }
        resolveFromHandle(handle);
    }
}

void IdentityResolver::resolveFromHandle(const QString &handle)
{
    resolveHandle(handle, [=](const QString &did) {
        if (did.isEmpty()) {
            finish(false, "Failed to resolve handle",
                   QString("Could not resolve '%1' to DID.").arg(handle));
            return;
        }
        resolveDidDocument(did,
                           [=](bool success, const AtProtocolType::DirectoryPlcDefs::DidDoc &doc) {
                               if (!success) {
                                   finish(false, "Failed to resolve DID",
                                          QString("Could not get DID document of '%1'.").arg(did));
                               } else if (handleInDidDocument(doc) != handle) {
                                   // ハンドル->DIDだけでなくDIDドキュメントもハンドルを主張していること
                                   finish(false, "Invalid identity",
                                          QString("DID document of '%1' does not claim '%2'.")
                                                  .arg(did, handle));
                               } else if (applyDidDocument(did, doc)) {
                                   m_handle = handle;
                                   finish(true);
                               }
                           });
    });
}

void IdentityResolver::resolveFromDid(const QString &did)
{
    resolveDidDocument(did, [=](bool success, const AtProtocolType::DirectoryPlcDefs::DidDoc &doc) {
        if (!success) {
            finish(false, "Failed to resolve DID",
                   QString("Could not get DID document of '%1'.").arg(did));
            return;
        }
        if (!applyDidDocument(did, doc)) {
            return;
        }
        const QString handle = handleInDidDocument(doc);
        if (!isValidHandle(handle)) {
            // ハンドルが無効でもDIDでの認証は続けられる
            finish(true);
            return;
        }
        resolveHandle(handle, [=](const QString &resolved_did) {
            if (resolved_did == did) {
                m_handle = handle;
            } else {
                qDebug().noquote() << "Handle in DID document is not verified :" << handle;
            }
            finish(true);
        });
    });
}

void IdentityResolver::resolveHandle(const QString &handle, DidCallback callback)
{
    // DNS TXT -> HTTPS well-known -> resolveHandle(サービス) の順
    resolveHandleByDns(handle, [=](const QString &did) {
        if (!did.isEmpty()) {
            callback(did);
            return;
        }
        resolveHandleByWellKnown(handle, [=](const QString &did) {
            if (!did.isEmpty()) {
                callback(did);
                return;
            }
            resolveHandleByService(handle, callback);
        });
    });
}

void IdentityResolver::resolveHandleByDns(const QString &handle, DidCallback callback)
{
    QDnsLookup *lookup =
            new QDnsLookup(QDnsLookup::TXT, QStringLiteral("_atproto.") + handle, this);
    QTimer *timer = new QTimer(lookup);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, lookup, &QDnsLookup::abort);
    connect(lookup, &QDnsLookup::finished, this, [=]() {
        timer->stop();
        QStringList dids;
        if (lookup->error() == QDnsLookup::NoError) {
            for (const auto &record : lookup->textRecords()) {
                const QString value = QString::fromUtf8(record.values().join());
                if (value.startsWith("did=")) {
                    dids.append(value.mid(4).trimmed());
                }
            }
        } else {
            qDebug().noquote() << "DNS lookup :" << lookup->name() << lookup->errorString();
        }
        lookup->deleteLater();
        // 有効なレコードが1つだけのときに採用する
        if (dids.length() == 1 && isValidDid(dids.first())) {
            callback(dids.first());
        } else {
            callback(QString());
        }
    });
    timer->start(m_dnsTimeout);
    lookup->lookup();
}

void IdentityResolver::resolveHandleByWellKnown(const QString &handle, DidCallback callback)
{
    AtProtocolInterface::AccountData account;
    account.service = QStringLiteral("https://") + handle;

    WellKnownAtprotoDid *well_known = new WellKnownAtprotoDid(this);
    connect(well_known, &WellKnownAtprotoDid::finished, this, [=](bool success) {
        const QString did = well_known->did();
        well_known->deleteLater();
        callback((success && isValidDid(did)) ? did : QString());
    });
    well_known->setAccount(account);
    well_known->atprotoDid();
}

void IdentityResolver::resolveHandleByService(const QString &handle, DidCallback callback)
{
    if (m_handleResolutionService.isEmpty()) {
        callback(QString());
        return;
    }
    AtProtocolInterface::AccountData account;
    account.service = m_handleResolutionService;

    ComAtprotoIdentityResolveHandle *resolver = new ComAtprotoIdentityResolveHandle(this);
    connect(resolver, &ComAtprotoIdentityResolveHandle::finished, this, [=](bool success) {
        const QString did = resolver->did();
        resolver->deleteLater();
        callback((success && isValidDid(did)) ? did : QString());
    });
    resolver->setAccount(account);
    resolver->resolveHandle(handle);
}

void IdentityResolver::resolveDidDocument(const QString &did, DidDocCallback callback)
{
    AtProtocolInterface::AccountData account;
    QString endpoint;
    if (did.startsWith("did:plc:")) {
        account.service = m_plcDirectory;
        endpoint = did;
    } else if (did.startsWith("did:web:")) {
        // atprotoのdid:webはホスト名のみ
        const QString host = did.mid(8);
        if (host.contains(':') || host.contains('%') || host.contains('/')) {
            callback(false, AtProtocolType::DirectoryPlcDefs::DidDoc());
            return;
        }
        account.service = QString("https://%1/.well-known").arg(host);
        endpoint = QStringLiteral("did.json");
    } else {
        callback(false, AtProtocolType::DirectoryPlcDefs::DidDoc());
        return;
    }

    DirectoryPlc *directory = new DirectoryPlc(this);
    connect(directory, &DirectoryPlc::finished, this, [=](bool success) {
        const AtProtocolType::DirectoryPlcDefs::DidDoc doc = directory->didDoc();
        directory->deleteLater();
        callback(success, doc);
    });
    directory->setAccount(account);
    directory->directory(endpoint);
}

bool IdentityResolver::applyDidDocument(const QString &did,
                                        const AtProtocolType::DirectoryPlcDefs::DidDoc &doc)
{
    if (doc.id != did) {
        finish(false, "Invalid identity",
               QString("DID document id(%1) does not match '%2'.").arg(doc.id, did));
        return false;
    }
    for (const auto &service : doc.service) {
        if ((service.id == "#atproto_pds" || service.id == did + "#atproto_pds")
            && service.type == "AtprotoPersonalDataServer") {
            const QUrl url(service.serviceEndpoint);
            if (url.isValid() && (url.scheme() == "https" || url.scheme() == "http")
                && !url.host().isEmpty()) {
                m_did = did;
                m_pdsEndpoint = service.serviceEndpoint;
                return true;
            }
        }
    }
    finish(false, "Invalid identity", QString("DID document of '%1' has no PDS.").arg(did));
    return false;
}

void IdentityResolver::finish(bool success, const QString &code, const QString &message)
{
    if (!success) {
        qDebug().noquote() << code << message;
        m_did.clear();
        m_handle.clear();
        m_pdsEndpoint.clear();
        emit errorOccurred(code, message);
    }
    emit finished(success);
}

QString IdentityResolver::handleInDidDocument(const AtProtocolType::DirectoryPlcDefs::DidDoc &doc)
{
    // 最初のat://がハンドル
    for (const auto &aka : doc.alsoKnownAs) {
        if (aka.startsWith("at://")) {
            return normalizeHandle(aka.mid(5));
        }
    }
    return QString();
}

QString IdentityResolver::normalizeHandle(const QString &handle)
{
    return handle.trimmed().toLower();
}

bool IdentityResolver::isValidHandle(const QString &handle)
{
    static const QRegularExpression regex(
            QStringLiteral("^([a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?\\.)+"
                           "[a-zA-Z]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?$"));
    static const QStringList disallowed_tlds = QStringList() << ".alt"
                                                             << ".arpa"
                                                             << ".example"
                                                             << ".internal"
                                                             << ".invalid"
                                                             << ".local"
                                                             << ".localhost"
                                                             << ".onion";
    if (handle.isEmpty() || handle.length() > 253 || !regex.match(handle).hasMatch()) {
        return false;
    }
    for (const auto &tld : disallowed_tlds) {
        if (handle.endsWith(tld, Qt::CaseInsensitive)) {
            return false;
        }
    }
    return true;
}

bool IdentityResolver::isValidDid(const QString &did)
{
    static const QRegularExpression regex(
            QStringLiteral("^did:[a-z]+:[a-zA-Z0-9._:%-]*[a-zA-Z0-9._-]$"));
    return (did.length() <= 2048 && regex.match(did).hasMatch()
            && (did.startsWith("did:plc:") || did.startsWith("did:web:")));
}

QString IdentityResolver::did() const
{
    return m_did;
}

QString IdentityResolver::handle() const
{
    return m_handle;
}

QString IdentityResolver::pdsEndpoint() const
{
    return m_pdsEndpoint;
}

QString IdentityResolver::handleResolutionService() const
{
    return m_handleResolutionService;
}

void IdentityResolver::setHandleResolutionService(const QString &newHandleResolutionService)
{
    m_handleResolutionService = newHandleResolutionService;
}

QString IdentityResolver::plcDirectory() const
{
    return m_plcDirectory;
}

void IdentityResolver::setPlcDirectory(const QString &newPlcDirectory)
{
    m_plcDirectory = newPlcDirectory;
}

int IdentityResolver::dnsTimeout() const
{
    return m_dnsTimeout;
}

void IdentityResolver::setDnsTimeout(int msec)
{
    m_dnsTimeout = msec;
}
