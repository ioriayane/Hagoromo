#include "sitemochottarticlegetarticle.h"

#include <QJsonDocument>
#include <QUrlQuery>

namespace AtProtocolInterface {

SiteMochottArticleGetArticle::SiteMochottArticleGetArticle(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void SiteMochottArticleGetArticle::getArticle(const QString &uri)
{
    QUrlQuery url_query;
    url_query.addQueryItem(QStringLiteral("uri"), uri);

    get(QStringLiteral("xrpc/site.mochott.article.getArticle"), url_query, false);
}

const QJsonObject &SiteMochottArticleGetArticle::article() const
{
    return m_article;
}

bool SiteMochottArticleGetArticle::parseJson(bool success, const QString reply_json)
{
    const QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    m_article = json_doc.object().value("article").toObject();
    return success && !m_article.isEmpty();
}

}
