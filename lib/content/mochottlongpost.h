#ifndef MOCHOTTLONGPOST_H
#define MOCHOTTLONGPOST_H

#include "atprotocol/lexicons.h"
#include "atprotocol/accessatprotocol.h"

#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>

// mochottの長文投稿(site.mochott.longPost)を読み書きするための処理
namespace MochottLongPost {

struct Reference
{
    QString uri;
    QString cid;
    QString rkey;

    bool isValid() const { return !rkey.isEmpty(); }
};

// ポストの冒頭部分。末尾の「もっと見る」のリンクを取り除いたもの
struct Opening
{
    QString text;
    QList<AtProtocolType::AppBskyRichtextFacet::Main> facets;
    QString article_url; // 末尾のリンクが見つからないときは空
};

struct Article
{
    bool valid = false;
    bool edited = false; // ポストが参照するCIDと記事のCIDが異なる(投稿後に編集された)
    QString text; // Text.StyledTextで表示する本文
    QString text_plain;
    // ポストのembedと重複しない画像
    QStringList images;
    QStringList images_alt;
    QStringList images_ratio; // 全画像のサイズが分かるときだけ入る(高さ/幅)
};

// 記事がポストの投稿者のものでなければ無効なReferenceを返す
Reference parseReference(const AtProtocolType::AppBskyFeedPost::Main &record,
                         const QString &author_did);

Opening parseOpening(const AtProtocolType::AppBskyFeedPost::Main &record, const QString &rkey);

// site.mochott.article.getArticleのarticleからArticleを作る
// 表示できないノードやマークが含まれるときはvalidがfalseになる
Article parseArticle(const QJsonObject &article, const Reference &reference,
                     const AtProtocolType::AppBskyFeedPost::Main &record,
                     const QString &author_did);

QString makeArticleUrl(const QString &handle, const QString &rkey);

// ---- 書く側 ----

const int POST_MAX_GRAPHEMES = 300;
const int POST_MAX_BYTES = 3000;
const int ARTICLE_MAX_BYTES = 100000;
const QString ARTICLE_VIA = QStringLiteral("tech.relog.hagoromo");

// Blueskyのポストとして投稿する冒頭部分
struct PostOpening
{
    QString body; // 本文から切り出した部分（「…」と「もっと見る」を含まない）
    QString text; // ポストの本文
    QList<AtProtocolType::AppBskyRichtextFacet::Main> facets;
};

struct ArticleSource
{
    QString did;
    QString profile; // getPublicationのpublication.uri
    QString rkey;
    QString created_at;
    QString text;
    QList<AtProtocolType::AppBskyRichtextFacet::Main> facets;
    QString description;
    // ポストの添付(画像・リンクカード・引用など)
    AtProtocolInterface::PostRecordData attachments;
};

// 本文をポストの上限に収まるように切り出し、末尾に記事へのリンクを付ける
// facetの途中では切らない
PostOpening makeOpening(const QString &text,
                        const QList<AtProtocolType::AppBskyRichtextFacet::Main> &facets,
                        const QString &more_label, const QString &article_url);

QJsonObject makeArticleRecord(const ArticleSource &source);

QJsonObject makeDocumentRecord(const QJsonObject &article, const QString &site,
                               const AtProtocolType::ComAtprotoRepoStrongRef::Main &post);

}

#endif // MOCHOTTLONGPOST_H
