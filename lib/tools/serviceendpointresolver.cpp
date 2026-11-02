#include "serviceendpointresolver.h"
#include "tools/identityresolver.h"

ServiceEndpointResolver::ServiceEndpointResolver(QObject *parent) : QObject { parent } { }

void ServiceEndpointResolver::resolve(
        const AtProtocolInterface::AccountData &account, const QString &target_did,
        const QString &target_service_endpoint,
        std::function<void(bool success, const QString &service_endpoint)> callback)
{
    if (!target_service_endpoint.isEmpty()) {
        callback(true, target_service_endpoint);
        return;
    }
    if (!target_did.isEmpty() && target_did == account.did) {
        callback(true, QString());
        return;
    }
    if (!target_did.isEmpty() && target_did == m_resolvedDid) {
        callback(true, m_resolvedServiceEndpoint);
        return;
    }

    IdentityResolver *resolver = new IdentityResolver(this);
    connect(resolver, &IdentityResolver::errorOccurred, this,
            &ServiceEndpointResolver::errorOccurred);
    connect(resolver, &IdentityResolver::finished, this, [=](bool success) {
        if (success) {
            m_resolvedDid = target_did;
            m_resolvedServiceEndpoint = resolver->pdsEndpoint();
            callback(true, m_resolvedServiceEndpoint);
        } else {
            callback(false, QString());
        }
        resolver->deleteLater();
    });
    if (!plcDirectory().isEmpty()) {
        resolver->setPlcDirectory(plcDirectory());
    }
    resolver->resolvePdsEndpoint(target_did);
}

QString ServiceEndpointResolver::plcDirectory() const
{
    return m_plcDirectory;
}

void ServiceEndpointResolver::setPlcDirectory(const QString &newPlcDirectory)
{
    m_plcDirectory = newPlcDirectory;
}
