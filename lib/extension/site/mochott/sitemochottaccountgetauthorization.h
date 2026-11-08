#ifndef SITEMOCHOTTACCOUNTGETAUTHORIZATION_H
#define SITEMOCHOTTACCOUNTGETAUTHORIZATION_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

// アカウントがmochottと連携済みか(PDS経由でmochottのAppViewに問い合わせる)
class SiteMochottAccountGetAuthorization : public AccessAtProtocol
{
public:
    explicit SiteMochottAccountGetAuthorization(QObject *parent = nullptr);

    void getAuthorization();

    bool authorized() const;
    const QString &authorizeUrl() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    bool m_authorized;
    QString m_authorizeUrl;
};

}

#endif // SITEMOCHOTTACCOUNTGETAUTHORIZATION_H
