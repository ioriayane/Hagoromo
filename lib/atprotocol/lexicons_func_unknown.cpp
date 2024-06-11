#ifndef LEXICONS_FUNC_UNKNOWN_CPP
#define LEXICONS_FUNC_UNKNOWN_CPP

#include "lexicons_func.h"
#include "lexicons_func_unknown.h"
#include "atprotocol/app/bsky/actor/appbskyactorgetprofiles.h"

#include <QCborMap>
#include <QCborArray>

using AtProtocolInterface::AppBskyActorGetProfiles;

namespace AtProtocolType {
namespace LexiconsTypeUnknown {

struct MentionData
{
    int start = -1;
    int end = -1;
};

void copyUnknown(const QJsonObject &src, QVariant &dest)
{
    if (src.isEmpty())
        return;

    QString type = src.value("$type").toString();
    if (type == QStringLiteral("app.bsky.feed.post")) {
        AppBskyFeedPost::Main record;
        AppBskyFeedPost::copyMain(src, record);
        dest.setValue(record);
    } else if (type == QStringLiteral("app.bsky.feed.like")) {
        AppBskyFeedLike::Main record;
        AppBskyFeedLike::copyMain(src, record);
        dest.setValue(record);
    } else if (type == QStringLiteral("app.bsky.feed.repost")) {
        AppBskyFeedRepost::Main record;
        AppBskyFeedRepost::copyMain(src, record);
        dest.setValue(record);
    } else if (type == QStringLiteral("app.bsky.graph.listitem")) {
        AppBskyGraphListitem::Main record;
        AppBskyGraphListitem::copyMain(src, record);
        dest.setValue(record);
    } else if (type == QStringLiteral("app.bsky.actor.profile")) {
        AppBskyActorProfile::Main record;
        AppBskyActorProfile::copyMain(src, record);
        dest.setValue(record);
    } else if (type == QStringLiteral("app.bsky.graph.list")) {
        AppBskyGraphList::Main record;
        AppBskyGraphList::copyMain(src, record);
        dest.setValue(record);
    } else if (type == QStringLiteral("app.bsky.feed.threadgate")) {
        AppBskyFeedThreadgate::Main record;
        AppBskyFeedThreadgate::copyMain(src, record);
        dest.setValue(record);
    } else if (type == QStringLiteral("com.whtwnd.blog.entry")) {
        ComWhtwndBlogEntry::Main record;
        ComWhtwndBlogEntry::copyMain(src, record);
        dest.setValue(record);
    }
}

void copyBlob(const QJsonObject &src, Blob &dest)
{
    if (src.isEmpty())
        return;
    if (src.value("$type").toString() != "blob")
        return;

    dest.cid = src.value("ref").toObject().value("$link").toString();
    dest.alt = src.value("alt").toString();
    dest.mimeType = src.value("mimeType").toString();
    dest.size = src.value("size").toInt();
}

void copyString(const QJsonValue &src, QString &dest)
{
    dest = src.toString();
}

void copyStringList(const QJsonArray &src, QStringList &dest)
{
    for (const auto &value : src) {
        dest.append(value.toString());
    }
}

void copyBool(const QJsonValue &src, bool &dest)
{
    dest = src.toBool(false);
}

void copyInt(const QJsonValue &src, int &dest)
{
    dest = src.toInt(0);
}

QStringList copyImagesFromPostView(const AppBskyFeedDefs::PostView &post, const CopyImageType type)
{
    if (post.embed_type == AppBskyFeedDefs::PostViewEmbedType::embed_AppBskyEmbedImages_View) {
        QStringList images;
        for (const auto &image : post.embed_AppBskyEmbedImages_View.images) {
            if (type == CopyImageType::Thumb)
                images.append(image.thumb);
            else if (type == CopyImageType::FullSize)
                images.append(image.fullsize);
            else if (type == CopyImageType::Alt)
                images.append(image.alt);
        }
        return images;
    } else if (post.embed_type
                       == AppBskyFeedDefs::PostViewEmbedType::embed_AppBskyEmbedRecordWithMedia_View
               && post.embed_AppBskyEmbedRecordWithMedia_View.media_type
                       == AppBskyEmbedRecordWithMedia::ViewMediaType::
                               media_AppBskyEmbedImages_View) {
        QStringList images;
        for (const auto &image :
             post.embed_AppBskyEmbedRecordWithMedia_View.media_AppBskyEmbedImages_View.images) {
            if (type == CopyImageType::Thumb)
                images.append(image.thumb);
            else if (type == CopyImageType::FullSize)
                images.append(image.fullsize);
            else if (type == CopyImageType::Alt)
                images.append(image.alt);
        }
        return images;
    } else {
        return QStringList();
    }
}

QStringList copyImagesFromRecord(const AppBskyEmbedRecord::ViewRecord &record,
                                 const CopyImageType type)
{
    // unionの配列で複数種類を混ぜられる
    QStringList images;
    for (const auto &view : record.embeds_AppBskyEmbedImages_View) {
        for (const auto &image : view.images) {
            if (type == CopyImageType::Thumb)
                images.append(image.thumb);
            else if (type == CopyImageType::FullSize)
                images.append(image.fullsize);
            else if (type == CopyImageType::Alt)
                images.append(image.alt);
        }
    }
    for (const auto &view : record.embeds_AppBskyEmbedRecordWithMedia_View) {
        if (view.media_type
            == AppBskyEmbedRecordWithMedia::ViewMediaType::media_AppBskyEmbedImages_View) {
            for (const auto &image : view.media_AppBskyEmbedImages_View.images) {
                if (type == CopyImageType::Thumb)
                    images.append(image.thumb);
                else if (type == CopyImageType::FullSize)
                    images.append(image.fullsize);
                else if (type == CopyImageType::Alt)
                    images.append(image.alt);
            }
        }
    }
    return images;
}

QStringList copyTagsFromFacets(const QList<AppBskyRichtextFacet::Main> &facets)
{
    QStringList tags;
    for (const auto &facet : facets) {
        for (const auto &tag : facet.features_Tag) {
            if (!tag.tag.isEmpty()) {
                tags.append(tag.tag);
            }
        }
    }
    return tags;
}

bool checkPartialMatchLanguage(const QStringList &langs)
{
    for (const auto &lang : langs) {
        QLocale locale(lang);
        if (locale.language() == QLocale::Japanese) {
            return true;
        }
    }
    return false;
}

QString copyRecordText(const QVariant &value)
{
    const AppBskyFeedPost::Main record =
            LexiconsTypeUnknown::fromQVariant<AppBskyFeedPost::Main>(value);
    return applyFacetsTo(record.text, record.facets);
}

QString formatDateTime(const QString &value, const bool is_long)
{
    if (is_long)
        return QDateTime::fromString(value, Qt::ISODateWithMs)
                .toLocalTime()
                .toString("yyyy/MM/dd hh:mm:ss");
    else
        return QDateTime::fromString(value, Qt::ISODateWithMs)
                .toLocalTime()
                .toString("MM/dd hh:mm");
}

void makeFacets(
        QObject *parent, AtProtocolInterface::AccountData account, const QString &text,
        std::function<void(const QList<AtProtocolType::AppBskyRichtextFacet::Main> &)> callback)
{

    QMultiMap<QString, MentionData> mention;
    QList<AtProtocolType::AppBskyRichtextFacet::Main> facets;
    QRegularExpression rx_facet = QRegularExpression(QString("(?:%1)|(?:%2)|(?:%3)")
                                                             .arg(REG_EXP_URL, REG_EXP_MENTION)
                                                             .arg(REG_EXP_HASH_TAG));

    QRegularExpressionMatch match = rx_facet.match(text);
    if (!match.capturedTexts().isEmpty()) {
        QString temp;
        int pos;
        int byte_start = 0;
        int byte_end = 0;
        while ((pos = match.capturedStart()) != -1) {
            byte_start = text.left(pos).toUtf8().length();
            temp = match.captured();
            byte_end = byte_start + temp.toUtf8().length();

            int trimmed_offset = 0;
            QString trimmed_temp = temp.trimmed();
            int temp_pos = temp.indexOf(trimmed_temp);
            int temp_diff_len = temp.length() - trimmed_temp.length();
            if (temp_diff_len > 0) {
                // 前後の空白を消す
                // 今のところhashtagだけここにくる可能性がある
                byte_start = text.left(pos + temp_pos).toUtf8().length();
                byte_end = byte_start + trimmed_temp.toUtf8().length();
                temp = trimmed_temp;
                if (temp_diff_len == 2 || (temp_diff_len == 1 && temp_pos == 0)) {
                    trimmed_offset = 1;
                }
            }
            if (temp.startsWith("@")) {
                temp.remove("@");
                MentionData position;
                position.start = byte_start;
                position.end = byte_end;
                mention.insert(temp, position);
            } else if (temp.startsWith("#")) {
                AppBskyRichtextFacet::Main facet;
                facet.index.byteStart = byte_start;
                facet.index.byteEnd = byte_end;
                AppBskyRichtextFacet::Tag tag;
                tag.tag = temp.mid(1);
                facet.features_type = AppBskyRichtextFacet::MainFeaturesType::features_Tag;
                facet.features_Tag.append(tag);
                facets.append(facet);
            } else {
                AppBskyRichtextFacet::Main facet;
                facet.index.byteStart = byte_start;
                facet.index.byteEnd = byte_end;
                AppBskyRichtextFacet::Link link;
                link.uri = temp;
                facet.features_type = AppBskyRichtextFacet::MainFeaturesType::features_Link;
                facet.features_Link.append(link);
                facets.append(facet);
            }

            match = rx_facet.match(text, pos + match.capturedLength() - trimmed_offset);
        }

        if (!mention.isEmpty()) {
            QStringList ids;
            for (const auto &key : mention.keys()) {
                if (!ids.contains(key)) {
                    ids.append(key);
                }
            }

            AppBskyActorGetProfiles *profiles = new AppBskyActorGetProfiles(parent);
            QObject::connect(profiles, &AppBskyActorGetProfiles::finished, [=](bool success) {
                QList<AtProtocolType::AppBskyRichtextFacet::Main> facets2(facets);
                if (success) {
                    for (const auto &item : qAsConst(profiles->profilesList())) {
                        QString handle = item.handle;
                        handle.remove("@");
                        if (mention.contains(handle)) {
                            const QList<MentionData> positions = mention.values(handle);
                            for (const auto &position : positions) {
                                AppBskyRichtextFacet::Main facet;
                                facet.index.byteStart = position.start;
                                facet.index.byteEnd = position.end;
                                AppBskyRichtextFacet::Mention mention;
                                mention.did = item.did;
                                facet.features_type =
                                        AppBskyRichtextFacet::MainFeaturesType::features_Mention;
                                facet.features_Mention.append(mention);
                                facets2.append(facet);
                            }
                        }
                    }
                }
                callback(facets2);
                profiles->deleteLater();
            });
            profiles->setAccount(account);
            profiles->getProfiles(ids);
        } else {
            // mentionがないときは直接戻る
            callback(facets);
        }
    } else {
        // uriもmentionがないときは直接戻る
        callback(facets);
    }
}

void insertFacetsJson(QJsonObject &parent, const QList<AppBskyRichtextFacet::Main> &facets)
{
    QJsonArray json_facets;
    for (const auto &facet : qAsConst(facets)) {
        QJsonObject json_facet;
        QJsonObject json_index;
        QJsonArray json_features;
        QJsonObject json_feature;

        json_index.insert("byteStart", facet.index.byteStart);
        json_index.insert("byteEnd", facet.index.byteEnd);
        if (facet.features_type == AppBskyRichtextFacet::MainFeaturesType::features_Link
            && !facet.features_Link.isEmpty()) {
            json_feature.insert("uri", facet.features_Link.first().uri);
            json_feature.insert("$type", "app.bsky.richtext.facet#link");
        } else if (facet.features_type == AppBskyRichtextFacet::MainFeaturesType::features_Mention
                   && !facet.features_Mention.isEmpty()) {
            json_facet.insert("$type", "app.bsky.richtext.facet");
            json_feature.insert("did", facet.features_Mention.first().did);
            json_feature.insert("$type", "app.bsky.richtext.facet#mention");
        } else if (facet.features_type == AppBskyRichtextFacet::MainFeaturesType::features_Tag
                   && !facet.features_Tag.isEmpty()) {
            json_feature.insert("tag", facet.features_Tag.first().tag);
            json_feature.insert("$type", "app.bsky.richtext.facet#tag");
        }
        json_facet.insert("index", json_index);
        json_features.append(json_feature);
        json_facet.insert("features", json_features);
        json_facets.append(json_facet);
    }
    if (!json_facets.isEmpty()) {
        parent.insert("facets", json_facets);
    }
}

QString applyFacetsTo(const QString &text, const QList<AppBskyRichtextFacet::Main> &text_facets)
{
    if (text_facets.isEmpty()) {
        return QString(text).toHtmlEscaped().replace("\n", "<br/>");
    } else {
        QByteArray text_ba = text.toUtf8();
        QString text;
        int pos_start = 0;
        int pos_end = 0;
        int pos_prev_end = 0;
        QList<AppBskyRichtextFacet::Main> facets = text_facets;
        for (int i = 0; i < facets.length() - 1; i++) {
            for (int j = i + 1; j < facets.length(); j++) {
                if (facets.at(i).index.byteStart > facets.at(j).index.byteStart) {
                    facets.swapItemsAt(i, j);
                }
            }
        }
        for (const auto &part : facets) {
            pos_start = part.index.byteStart;
            pos_end = part.index.byteEnd;

            // 0 : [a]b(c)
            // 1 : [(abc)]
            // 2 : [a(bc)]
            // 3 : [a(b)c]
            // 4 : [a(b]c)

            if (pos_start < pos_prev_end) {
                // 前回の終了位置より開始が前（つまり、互い違いになっている -> [あ(い]う)の状態）
                if (pos_end <= pos_prev_end) {
                    // 1 : [(abc)]
                    // 2 : [a(bc)]
                    // 3 : [a(b)c]
                } else {
                    // 4 : [a(b]c)
                    text += QString(text_ba.mid(pos_prev_end, pos_end - pos_prev_end))
                                    .toHtmlEscaped()
                                    .replace("\n", "<br/>");
                    pos_prev_end = pos_end;
                }
            } else {
                // 0 : [a]b(c)
                if (pos_start > pos_prev_end) {
                    text += QString(text_ba.mid(pos_prev_end, pos_start - pos_prev_end))
                                    .toHtmlEscaped()
                                    .replace("\n", "<br/>");
                }
                QString display_url =
                        QString::fromUtf8(text_ba.mid(pos_start, pos_end - pos_start));
                if (!part.features_Link.isEmpty()) {
                    text += QString("<a href=\"%1\">%2</a>")
                                    .arg(part.features_Link.first().uri, display_url);
                } else if (!part.features_Mention.isEmpty()) {
                    text += QString("<a href=\"%1\">%2</a>")
                                    .arg(part.features_Mention.first().did, display_url);
                } else if (!part.features_Tag.isEmpty()) {
                    text += QString("<a href=\"search://%1\">%2</a>")
                                    .arg(part.features_Tag.first().tag, display_url);
                } else {
                    text += QString(text_ba.mid(pos_start, pos_end - pos_start))
                                    .toHtmlEscaped()
                                    .replace("\n", "<br/>");
                }
                pos_prev_end = pos_end;
            }
        }
        if (pos_prev_end < (text_ba.length() - 1)) {
            text += QString(text_ba.mid(pos_prev_end)).toHtmlEscaped().replace("\n", "<br/>");
        }

        return text;
    }
}
}

namespace ComAtprotoSyncSubscribeRepos {

void copyRepoOp(const QCborValue &src, RepoOp &dest)
{
    if (src.isMap()) {
        const auto &src_m = src.toMap();
        dest.action = src_m.value("action").toString();
        dest.path = src_m.value("path").toString();
        if (src_m.value("cid").isMap()) {
            dest.cid = src_m.value("cid").toMap().value("$link").toString();
        }
    }
}

void copyCommit(const QCborValue &src, Commit &dest)
{
    if (src.isMap()) {
        const auto &src_m = src.toMap();
        dest.seq = src_m.value("seq").toInteger();
        dest.tooBig = src_m.value("tooBig").toBool();
        dest.repo = src_m.value("repo").toString();
        if (src_m.value("commit").isMap()) {
            dest.commit = src_m.value("commit").toMap().value("$link").toString();
        }
        dest.rev = src_m.value("rev").toString();
        dest.since = src_m.value("since").toString();
        for (const auto &s : src_m.value("ops").toArray()) {
            RepoOp child;
            copyRepoOp(s, child);
            dest.ops.append(child);
        }
        dest.time = src_m.value("time").toString();
    }
}

void copyBlockHeader(const QCborValue &src, BlockHeader &dest)
{
    if (src.isMap()) {
        const auto &src_m = src.toMap();
        dest.op = src_m.value("op").toInteger(-1);
        dest.t = src_m.value("t").toString();
    } else {
        dest.op = -1;
        dest.t.clear();
    }
}

}

}

#endif // LEXICONS_FUNC_UNKNOWN_CPP
