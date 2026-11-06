#include "mochottoperator.h"
#include "extension/site/mochott/sitemochottarticlegetarticle.h"

using AtProtocolInterface::SiteMochottArticleGetArticle;

MochottOperator::MochottOperator(QObject *parent)
    : QObject { parent },
      m_serviceUrl(QStringLiteral("https://mochott.site")),
      m_articles(16 * 1024 * 1024) // 本文の文字数の合計の上限
{
}

MochottOperator *MochottOperator::getInstance()
{
    static MochottOperator instance;
    return &instance;
}

void MochottOperator::setServiceUrl(const QString &url)
{
    if (url.isEmpty())
        return;
    m_serviceUrl = url;
}

void MochottOperator::getArticle(const QString &cid, const MochottLongPost::Reference &reference,
                                 const AtProtocolType::AppBskyFeedPost::Main &record,
                                 const QString &author_did)
{
    if (cid.isEmpty() || !reference.isValid() || m_running.contains(cid)
        || m_articles.contains(cid)) {
        return;
    }
    m_running.insert(cid);
    m_failed.remove(cid);
    emit stateChanged(cid);

    auto get_article = new SiteMochottArticleGetArticle(this);
    connect(get_article, &SiteMochottArticleGetArticle::finished, this, [=](bool success) {
        MochottLongPost::Article article;
        if (success) {
            article = MochottLongPost::parseArticle(get_article->article(), reference, record,
                                                    author_did);
        }
        m_running.remove(cid);
        if (article.valid) {
            const int cost = article.text.length() + article.text_plain.length();
            m_articles.insert(cid, new MochottLongPost::Article(article), cost);
        } else {
            m_failed.insert(cid);
        }
        emit stateChanged(cid);
        get_article->deleteLater();
    });
    get_article->setService(m_serviceUrl);
    get_article->getArticle(reference.uri);
}

bool MochottOperator::running(const QString &cid) const
{
    return m_running.contains(cid);
}

bool MochottOperator::failed(const QString &cid) const
{
    return m_failed.contains(cid);
}

MochottLongPost::Article MochottOperator::article(const QString &cid) const
{
    const auto *cached = m_articles.object(cid);
    return cached != nullptr ? *cached : MochottLongPost::Article();
}
