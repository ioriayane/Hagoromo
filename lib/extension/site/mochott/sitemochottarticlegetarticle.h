#ifndef SITEMOCHOTTARTICLEGETARTICLE_H
#define SITEMOCHOTTARTICLEGETARTICLE_H

#include "atprotocol/accessatprotocol.h"

#include <QJsonObject>

namespace AtProtocolInterface {

// contentJSONには$typeが無く生成コードのunknownの処理では保持できないのでJSONのまま扱う
class SiteMochottArticleGetArticle : public AccessAtProtocol
{
public:
    explicit SiteMochottArticleGetArticle(QObject *parent = nullptr);

    void getArticle(const QString &uri);

    const QJsonObject &article() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QJsonObject m_article;
};

}

#endif // SITEMOCHOTTARTICLEGETARTICLE_H
