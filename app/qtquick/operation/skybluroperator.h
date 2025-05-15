#ifndef SKYBLUROPERATOR_H
#define SKYBLUROPERATOR_H

#include "atprotocol/accessatprotocol.h"

#include <QObject>

class SkyblurOperator : public QObject
{
    Q_OBJECT
public:
    explicit SkyblurOperator(QObject *parent = nullptr);

    static SkyblurOperator *getInstance();

    void restoreBluredText(const QString &cid, const QString &at_uri);
    QString getUnbluredText(const QString &cid) const;

    void setAccount(const QString &uuid);

signals:
    void finished(bool success, const QString &cid, const QString text);

private:
    AtProtocolInterface::AccountData account() const;
    void getServiceEndpoint(const QString &did,
                            std::function<void(const QString &service_endpoint)> callback);

    QHash<QString, QString> m_unbluredText; // QHash<cid, unblured>
    QString m_uuid;
};

#endif // SKYBLUROPERATOR_H
