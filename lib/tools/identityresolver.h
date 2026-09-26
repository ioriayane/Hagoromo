#ifndef IDENTITYRESOLVER_H
#define IDENTITYRESOLVER_H

#include <QObject>
#include <functional>

namespace AtProtocolType {
namespace DirectoryPlcDefs {
struct DidDoc;
}
}

// ハンドルまたはDIDからアカウントのDID/ハンドル/PDSを解決する
// ハンドルとDIDドキュメントは双方向で検証する
class IdentityResolver : public QObject
{
    Q_OBJECT
public:
    explicit IdentityResolver(QObject *parent = nullptr);

    void resolve(const QString &identifier);

    QString did() const;
    // 検証済みのハンドル(検証できない場合は空)
    QString handle() const;
    QString pdsEndpoint() const;

    // DNS/HTTPSで解決できないときに com.atproto.identity.resolveHandle で問い合わせるサービス
    QString handleResolutionService() const;
    void setHandleResolutionService(const QString &newHandleResolutionService);
    QString plcDirectory() const;
    void setPlcDirectory(const QString &newPlcDirectory);
    int dnsTimeout() const;
    void setDnsTimeout(int msec);

    static QString normalizeHandle(const QString &handle);
    static bool isValidHandle(const QString &handle);
    static bool isValidDid(const QString &did);

signals:
    void finished(bool success);
    void errorOccurred(const QString &code, const QString &message);

private:
    typedef std::function<void(const QString &did)> DidCallback;
    typedef std::function<void(bool success, const AtProtocolType::DirectoryPlcDefs::DidDoc &doc)>
            DidDocCallback;

    void resolveFromHandle(const QString &handle);
    void resolveFromDid(const QString &did);
    void resolveHandle(const QString &handle, DidCallback callback);
    void resolveHandleByDns(const QString &handle, DidCallback callback);
    void resolveHandleByWellKnown(const QString &handle, DidCallback callback);
    void resolveHandleByService(const QString &handle, DidCallback callback);
    void resolveDidDocument(const QString &did, DidDocCallback callback);
    bool applyDidDocument(const QString &did, const AtProtocolType::DirectoryPlcDefs::DidDoc &doc);
    void finish(bool success, const QString &code = QString(), const QString &message = QString());

    static QString handleInDidDocument(const AtProtocolType::DirectoryPlcDefs::DidDoc &doc);

    QString m_did;
    QString m_handle;
    QString m_pdsEndpoint;

    QString m_handleResolutionService;
    QString m_plcDirectory;
    int m_dnsTimeout;
};

#endif // IDENTITYRESOLVER_H
