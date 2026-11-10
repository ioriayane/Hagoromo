#ifndef SITEMOCHOTTPUBLICATIONGETPUBLICATION_H
#define SITEMOCHOTTPUBLICATIONGETPUBLICATION_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class SiteMochottPublicationGetPublication : public AccessAtProtocol
{
public:
    explicit SiteMochottPublicationGetPublication(QObject *parent = nullptr);

    void getPublication(const QString &did);

    // 記事のprofileに使う(形は決め打ちしない)
    const QString &uri() const;
    // site.standard.documentのsiteに使う。無ければdocumentは書かない
    const QString &standardPublication() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QString m_uri;
    QString m_standardPublication;
};

}

#endif // SITEMOCHOTTPUBLICATIONGETPUBLICATION_H
