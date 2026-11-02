#ifndef SERVICEENDPOINTRESOLVER_H
#define SERVICEENDPOINTRESOLVER_H

#include "atprotocol/accessatprotocol.h"

#include <QObject>
#include <functional>

// 他のアカウントのリポジトリ(com.atproto.repo.listRecordsなど)にアクセスするときの接続先を決める
// 他のアカウントのレコードは自分のPDSではなく、対象アカウントのPDSにある
class ServiceEndpointResolver : public QObject
{
    Q_OBJECT
public:
    explicit ServiceEndpointResolver(QObject *parent = nullptr);

    // 接続先の優先順
    //   1. target_service_endpointの指定
    //   2. 自分のリポジトリなら空(自分のPDSのまま)
    //   3. 対象のDIDドキュメントから解決したPDS(同じDIDは使い回す)
    // 解決できなかったときはsuccess=falseで、自分のPDSにはフォールバックしない
    void resolve(const AtProtocolInterface::AccountData &account, const QString &target_did,
                 const QString &target_service_endpoint,
                 std::function<void(bool success, const QString &service_endpoint)> callback);

    QString plcDirectory() const;
    void setPlcDirectory(const QString &newPlcDirectory);

signals:
    void errorOccurred(const QString &code, const QString &message);

private:
    QString m_resolvedDid;
    QString m_resolvedServiceEndpoint;
    QString m_plcDirectory;
};

#endif // SERVICEENDPOINTRESOLVER_H
