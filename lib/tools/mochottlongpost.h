#ifndef MOCHOTTLONGPOST_H
#define MOCHOTTLONGPOST_H

#include "atprotocol/lexicons.h"

#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>

// mochottの長文投稿(site.mochott.longPost)を読むための処理
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

}

#endif // MOCHOTTLONGPOST_H
