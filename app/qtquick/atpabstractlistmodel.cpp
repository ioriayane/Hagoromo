#include "atpabstractlistmodel.h"
#include "atprotocol/com/atproto/sync/comatprotosyncgetblob.h"
#include "atprotocol/lexicons_func_unknown.h"
#include "tools/labelerprovider.h"
#include "common.h"
#include "translator.h"

#include <QDesktopServices>
#include <QUrlQuery>

using namespace AtProtocolType;
using AtProtocolInterface::ComAtprotoSyncGetBlob;

AtpAbstractListModel::AtpAbstractListModel(QObject *parent)
    : QAbstractListModel { parent },
      m_contentFilterRefreshCounter(0),
      m_running(false),
      m_loadingInterval(5 * 60 * 1000),
      m_displayInterval(400),
      m_visibleContainingMutedWord(true),
      m_displayPinnedPost(false)
{
    connect(&m_timer, &QTimer::timeout, this, &AtpAbstractListModel::getLatest);
}

void AtpAbstractListModel::clear()
{
    if (!m_cidList.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, m_cidList.count() - 1);
        m_cidList.clear();
        endRemoveRows();
    }
    m_originalCidList.clear();
    m_translations.clear();
    m_mutedPosts.clear();
    m_cursor.clear();
    m_currentPinnedPost.clear();
    m_pinnedUriCid.clear();
    m_pinnedPost.clear();
}

AtProtocolInterface::AccountData AtpAbstractListModel::account() const
{
    return m_account;
}

void AtpAbstractListModel::setAccount(const QString &service, const QString &did,
                                      const QString &handle, const QString &email,
                                      const QString &accessJwt, const QString &refreshJwt)
{
    m_account.service = service;
    m_account.did = did;
    m_account.handle = handle;
    m_account.email = email;
    m_account.accessJwt = accessJwt;
    m_account.refreshJwt = refreshJwt;
}

void AtpAbstractListModel::translate(const QString &cid)
{
    // indexで指定しないと同じcidが複数含まれる場合に正しく対応できない
    // indexにすると処理を始めてから追加の読み込みがあるとズレる

    QString record_text = getRecordText(cid);
    if (record_text.isEmpty())
        return;
    int row = indexOf(cid);
    if (row == -1)
        return;

    Translator *translator = new Translator(this);
    if (!translator->validSettings()) {
        // 設定画無いときはGoogle翻訳へ飛ばす
        QUrl url("https://translate.google.com/");
        QUrlQuery query;
        query.addQueryItem("sl", "auto");
        if (!translator->targetLanguage().isEmpty()) {
            query.addQueryItem("tl", translator->targetLanguage().toLower());
        }
        query.addQueryItem("text", record_text);
        url.setQuery(query);
        QDesktopServices::openUrl(url);
    } else {
        connect(translator, &Translator::finished, [=](const QString text) {
            if (!text.isEmpty()) {
                m_translations[cid] = text;
                emit dataChanged(index(row), index(row));
            }
            translator->deleteLater();
        });
        m_translations[cid] = "Now translating ...";
        emit dataChanged(index(row), index(row));
        translator->translate(record_text);
    }
}

void AtpAbstractListModel::reflectVisibility()
{
    int prev_row = -1;
    for (const auto &cid : qAsConst(m_originalCidList)) {
        if (checkVisibility(cid)) {
            // 表示させる
            if (m_cidList.contains(cid)) {
                prev_row = m_cidList.indexOf(cid);
            } else {
                if (aggregateQueuedPosts(cid, true)) {
                    prev_row++;
                    beginInsertRows(QModelIndex(), prev_row, prev_row);
                    m_cidList.insert(prev_row, cid);
                    endInsertRows();
                }
            }
        } else {
            // 消す
            if (m_cidList.contains(cid)) {
                int r = m_cidList.indexOf(cid);
                beginRemoveRows(QModelIndex(), r, r);
                m_cidList.removeAt(r);
                endRemoveRows();
                prev_row = r - 1;
            }
        }
    }
}

bool AtpAbstractListModel::running() const
{
    return m_running;
}

void AtpAbstractListModel::setRunning(bool newRunning)
{
    if (m_running == newRunning)
        return;
    m_running = newRunning;
    emit runningChanged();
}

void AtpAbstractListModel::displayQueuedPosts()
{
    int interval = m_displayInterval;
    bool batch_mode = (m_originalCidList.isEmpty() || interval == 0);
    int top_index = hasPinnedPost() ? 1 : 0;

    while (!m_cuePost.isEmpty()) {
        const PostCueItem &post = m_cuePost.front();
        bool visible = checkVisibility(post.cid);
        if (!visible)
            interval = 0;

        if (m_originalCidList.contains(post.cid)) {
            if (post.reason_type == AppBskyFeedDefs::FeedViewPostReasonType::reason_ReasonRepost
                && (QDateTime::fromString(post.indexed_at, Qt::ISODateWithMs)
                    > post.reference_time)) {
                // repostのときはいったん消す（上に持っていく）
                int r;
                if (visible) {
                    r = m_cidList.indexOf(post.cid);
                    if (r > top_index) {
                        beginMoveRows(QModelIndex(), r, r, QModelIndex(), top_index);
                        m_cidList.move(r, top_index);
                        endMoveRows();
                    }
                }
                r = m_originalCidList.indexOf(post.cid);
                if (r > top_index) {
                    m_originalCidList.move(r, top_index);
                }
            } else {
                // リストは更新しないでデータのみ入れ替える
                // 更新をUIに通知
                // （取得できた範囲でしか更新できないのだけど・・・）
                interval = 0;
                int r = m_cidList.indexOf(post.cid);
                if (r >= 0) {
                    do {
                        if (visible) {
                            emit dataChanged(index(r), index(r));
                        } else {
                            beginRemoveRows(QModelIndex(), r, r);
                            m_cidList.removeAt(r);
                            endRemoveRows();
                        }
                        r = m_cidList.indexOf(post.cid, ++r);
                    } while (r >= 0);
                } else {
                    int r = searchInsertPosition(post.cid);
                    if (visible && r >= 0) {
                        // 復活させる
                        beginInsertRows(QModelIndex(), r, r);
                        m_cidList.insert(r, post.cid);
                        endInsertRows();
                    }
                }
            }
        } else {
            if (visible) {
                beginInsertRows(QModelIndex(), top_index, top_index);
                m_cidList.insert(top_index, post.cid);
                endInsertRows();
            }
            m_originalCidList.insert(top_index, post.cid);
        }

        m_cuePost.pop_front();

        if (!batch_mode)
            break;
    }

    if (!m_cuePost.isEmpty()) {
        QTimer::singleShot(interval, this, &AtpAbstractListModel::displayQueuedPosts);
    } else {
        finishedDisplayingQueuedPosts();
    }
}

void AtpAbstractListModel::displayQueuedPostsNext()
{
    while (!m_cuePost.isEmpty()) {
        const PostCueItem &post = m_cuePost.back();
        bool visible = checkVisibility(post.cid);

        if (m_originalCidList.contains(post.cid)) {
            if (post.reason_type == AppBskyFeedDefs::FeedViewPostReasonType::reason_ReasonRepost) {
                // 通常、repostのときはいったん消して上へ移動だけど
                // 続きの読み込みの時は下へ入れることになるので無視
            } else {
                // リストは更新しないでデータのみ入れ替える
                // 更新をUIに通知
                // （取得できた範囲でしか更新できないのだけど・・・）
                int r = m_cidList.indexOf(post.cid);
                if (r >= 0) {
                    if (visible) {
                        emit dataChanged(index(r), index(r));
                    } else {
                        beginRemoveRows(QModelIndex(), r, r);
                        m_cidList.removeAt(r);
                        endRemoveRows();
                    }
                } else {
                    int r = searchInsertPosition(post.cid);
                    if (visible && r >= 0) {
                        // 復活させる
                        beginInsertRows(QModelIndex(), r, r);
                        m_cidList.insert(r, post.cid);
                        endInsertRows();
                    }
                }
            }
        } else {
            if (visible) {
                beginInsertRows(QModelIndex(), m_cidList.count(), m_cidList.count());
                m_cidList.append(post.cid);
                endInsertRows();
            }
            m_originalCidList.append(post.cid);
        }

        m_cuePost.pop_back();
    }

    finishedDisplayingQueuedPosts();
}

int AtpAbstractListModel::searchInsertPosition(const QString &cid)
{
    int basis_pos = m_originalCidList.indexOf(cid);
    if (basis_pos < 0)
        return -1;
    if (basis_pos == 0)
        return 0;
    for (int i = basis_pos - 1; i >= 0; i--) {
        int pos = m_cidList.indexOf(m_originalCidList.at(i));
        if (pos >= 0)
            return pos + 1;
    }
    return -1;
}

void AtpAbstractListModel::updateContentFilterLabels(std::function<void()> callback)
{
    LabelerProvider *provider = LabelerProvider::getInstance();
    LabelerConnector *connector = new LabelerConnector(this);

    connect(connector, &LabelerConnector::finished, this, [=](bool success) {
        if (success) { }
        callback();
        connector->deleteLater();
    });
    provider->setAccount(account());
    provider->update(account(), connector, LabelerProvider::RefleshAuto);
}

ConfigurableLabelStatus AtpAbstractListModel::getContentFilterStatus(
        const QList<AtProtocolType::ComAtprotoLabelDefs::Label> &labels, const bool for_media) const
{
    for (const auto &label : labels) {
        ConfigurableLabelStatus status = visibilityBylabeler(label.val, for_media, label.src);
        if (status != ConfigurableLabelStatus::Show) {
            return status;
        }
    }
    return ConfigurableLabelStatus::Show;
}

bool AtpAbstractListModel::getContentFilterMatched(
        const QList<AtProtocolType::ComAtprotoLabelDefs::Label> &labels, const bool for_media) const
{
    return (getContentFilterStatus(labels, for_media) != ConfigurableLabelStatus::Show);
}

QString AtpAbstractListModel::getContentFilterMessage(
        const QList<AtProtocolType::ComAtprotoLabelDefs::Label> &labels, const bool for_media) const
{
    QString message;
    for (const auto &label : labels) {
        message = contentFilterMessage(label.val, for_media, label.src);
        if (!message.isEmpty()) {
            break;
        }
    }
    return message;
}

bool AtpAbstractListModel::getQuoteFilterMatched(
        const AtProtocolType::AppBskyFeedDefs::PostView &post) const
{
    if (!post.embed_AppBskyEmbedRecord_View.isNull()
        && post.embed_AppBskyEmbedRecord_View->record_type
                == AppBskyEmbedRecord::ViewRecordType::record_ViewRecord) {
        if (post.embed_AppBskyEmbedRecord_View->record_ViewRecord.author.viewer.muted)
            return true;

        if (post.embed_AppBskyEmbedRecord_View->record_ViewRecord.author.did != account().did) {
            // 引用されているポストが他人のポストのみ判断する（自分のものの場合は隠さない）
            if (getContentFilterStatus(post.embed_AppBskyEmbedRecord_View->record_ViewRecord.labels,
                                       false)
                == ConfigurableLabelStatus::Warning)
                return true;
            if (getContentFilterStatus(post.embed_AppBskyEmbedRecord_View->record_ViewRecord.labels,
                                       true)
                == ConfigurableLabelStatus::Warning)
                return true;
        }
    }
    if (!post.embed_AppBskyEmbedRecordWithMedia_View.record.isNull()
        && post.embed_AppBskyEmbedRecordWithMedia_View.record->record_type
                == AppBskyEmbedRecord::ViewRecordType::record_ViewRecord) {
        if (post.embed_AppBskyEmbedRecordWithMedia_View.record->record_ViewRecord.author.viewer
                    .muted)
            return true;
        if (post.embed_AppBskyEmbedRecordWithMedia_View.record->record_ViewRecord.author.did
            != account().did) {
            // 引用されているポストが他人のポストのみ判断する（自分のものの場合は隠さない）
            if (getContentFilterStatus(post.embed_AppBskyEmbedRecordWithMedia_View.record
                                               ->record_ViewRecord.labels,
                                       false)
                == ConfigurableLabelStatus::Warning)
                return true;
            if (getContentFilterStatus(post.embed_AppBskyEmbedRecordWithMedia_View.record
                                               ->record_ViewRecord.labels,
                                       true)
                == ConfigurableLabelStatus::Warning)
                return true;
        }
    }

    return false;
}

QStringList AtpAbstractListModel::getLabels(
        const QList<AtProtocolType::ComAtprotoLabelDefs::Label> &labels) const
{
    QStringList ret;
    for (const auto &label : labels) {
        ret.append(label.val);
    }
    return ret;
}

QStringList AtpAbstractListModel::getLaunguages(const QVariant &record) const
{
    return LexiconsTypeUnknown::fromQVariant<AppBskyFeedPost::Main>(record).langs;
}

QString AtpAbstractListModel::getVia(const QVariant &record) const
{
    return LexiconsTypeUnknown::fromQVariant<AppBskyFeedPost::Main>(record).via;
}

QVariant
AtpAbstractListModel::getExternalLinkItem(const AtProtocolType::AppBskyFeedDefs::PostView &post,
                                          const ExternalLinkRoles role) const
{
    bool is_external =
            post.embed_type == AppBskyFeedDefs::PostViewEmbedType::embed_AppBskyEmbedExternal_View;
    bool with_image = post.embed_AppBskyEmbedRecordWithMedia_View.media_type
            == AppBskyEmbedRecordWithMedia::ViewMediaType::media_AppBskyEmbedExternal_View;

    if (role == HasExternalLinkRole) {
        if (is_external) {
            return true;
        } else if (with_image) {
            return true;
        } else {
            return false;
        }
    } else if (role == ExternalLinkUriRole) {
        if (is_external) {
            return post.embed_AppBskyEmbedExternal_View.external.uri;
        } else if (with_image) {
            return post.embed_AppBskyEmbedRecordWithMedia_View.media_AppBskyEmbedExternal_View
                    .external.uri;
        } else {
            return QString();
        }
    } else if (role == ExternalLinkTitleRole) {
        if (is_external) {
            return post.embed_AppBskyEmbedExternal_View.external.title;
        } else if (with_image) {
            return post.embed_AppBskyEmbedRecordWithMedia_View.media_AppBskyEmbedExternal_View
                    .external.title;
        } else {
            return QString();
        }
    } else if (role == ExternalLinkDescriptionRole) {
        QString description;
        if (is_external) {
            description = post.embed_AppBskyEmbedExternal_View.external.description;
        } else if (with_image) {
            description = post.embed_AppBskyEmbedRecordWithMedia_View
                                  .media_AppBskyEmbedExternal_View.external.description;
        }
        return Common::truncateTo3lines(description);
    } else if (role == ExternalLinkThumbRole) {
        if (is_external) {
            return post.embed_AppBskyEmbedExternal_View.external.thumb;
        } else if (with_image) {
            return post.embed_AppBskyEmbedRecordWithMedia_View.media_AppBskyEmbedExternal_View
                    .external.thumb;
        } else {
            return QString();
        }
    }

    return QVariant();
}

QVariant
AtpAbstractListModel::getFeedGeneratorItem(const AtProtocolType::AppBskyFeedDefs::PostView &post,
                                           const FeedGeneratorRoles role) const
{
    bool has_record = !post.embed_AppBskyEmbedRecord_View.isNull();

    if (role == HasFeedGeneratorRole) {
        if (has_record)
            return post.embed_type
                    == AppBskyFeedDefs::PostViewEmbedType::embed_AppBskyEmbedRecord_View
                    && post.embed_AppBskyEmbedRecord_View->record_type
                    == AppBskyEmbedRecord::ViewRecordType::record_AppBskyFeedDefs_GeneratorView;
        else
            return false;
    } else if (role == FeedGeneratorUriRole) {
        if (has_record)
            return post.embed_AppBskyEmbedRecord_View->record_AppBskyFeedDefs_GeneratorView.uri;
        else
            return QString();
    } else if (role == FeedGeneratorCreatorHandleRole) {
        if (has_record)
            return post.embed_AppBskyEmbedRecord_View->record_AppBskyFeedDefs_GeneratorView.creator
                    .handle;
        else
            return QString();
    } else if (role == FeedGeneratorDisplayNameRole) {
        if (has_record)
            return post.embed_AppBskyEmbedRecord_View->record_AppBskyFeedDefs_GeneratorView
                    .displayName;
        else
            return QString();
    } else if (role == FeedGeneratorLikeCountRole) {
        if (has_record)
            return post.embed_AppBskyEmbedRecord_View->record_AppBskyFeedDefs_GeneratorView
                    .likeCount;
        else
            return QString();
    } else if (role == FeedGeneratorAvatarRole) {
        if (has_record)
            return post.embed_AppBskyEmbedRecord_View->record_AppBskyFeedDefs_GeneratorView.avatar;
        else
            return QString();
    }
    return QVariant();
}

QVariant
AtpAbstractListModel::getListLinkItem(const AtProtocolType::AppBskyFeedDefs::PostView &post,
                                      const ListLinkRoles role) const
{
    bool has_record = !post.embed_AppBskyEmbedRecord_View.isNull();

    if (role == HasListLinkRole) {
        if (has_record)
            return post.embed_type
                    == AppBskyFeedDefs::PostViewEmbedType::embed_AppBskyEmbedRecord_View
                    && post.embed_AppBskyEmbedRecord_View->record_type
                    == AppBskyEmbedRecord::ViewRecordType::record_AppBskyGraphDefs_ListView;
        else
            return false;
    } else if (role == ListLinkUriRole) {
        if (has_record)
            return post.embed_AppBskyEmbedRecord_View->record_AppBskyGraphDefs_ListView.uri;
        else
            return QString();
    } else if (role == ListLinkCreatorHandleRole) {
        if (has_record
            && !post.embed_AppBskyEmbedRecord_View->record_AppBskyGraphDefs_ListView.creator
                        .isNull())
            return post.embed_AppBskyEmbedRecord_View->record_AppBskyGraphDefs_ListView.creator
                    ->handle;
        else
            return QString();
    } else if (role == ListLinkDisplayNameRole) {
        if (has_record)
            return post.embed_AppBskyEmbedRecord_View->record_AppBskyGraphDefs_ListView.name;
        else
            return QString();
    } else if (role == ListLinkDescriptionRole) {
        if (has_record)
            return Common::truncateTo3lines(post.embed_AppBskyEmbedRecord_View
                                                    ->record_AppBskyGraphDefs_ListView.description);
        else
            return QString();
    } else if (role == ListLinkAvatarRole) {
        if (has_record)
            return post.embed_AppBskyEmbedRecord_View->record_AppBskyGraphDefs_ListView.avatar;
        else
            return QString();
    }
    return QVariant();
}

QVariant
AtpAbstractListModel::getThreadGateItem(const AtProtocolType::AppBskyFeedDefs::PostView &post,
                                        const ThreadGateRoles role) const
{
    AppBskyFeedThreadgate::Main record =
            LexiconsTypeUnknown::fromQVariant<AppBskyFeedThreadgate::Main>(post.threadgate.record);

    QString type;
    QStringList rules;
    if (post.threadgate.uri.isEmpty()) {
        type = "everybody";
    } else {
        if (!record.allow_MentionRule.isEmpty()) {
            rules.append("mentioned");
        }
        if (!record.allow_FollowingRule.isEmpty()) {
            rules.append("followed");
        }
        for (const auto &item : record.allow_ListRule) {
            rules.append(item.list);
        }
        if (rules.isEmpty()) {
            type = "nobody";
        } else {
            type = "choice";
        }
    }

    if (role == ThreadGateUriRole) {
        return post.threadgate.uri;
    } else if (role == ThreadGateTypeRole) {
        return type;
    } else if (role == ThreadGateRulesRole) {
        return rules;
    }
    return QVariant();
}

void AtpAbstractListModel::updateThreadGateItem(AtProtocolType::AppBskyFeedDefs::PostView &post,
                                                const ThreadGateRoles role, const QVariant &value)
{
    if (role == ThreadGateUriRole) {
        post.threadgate.uri = value.toString();
    } else if (role == ThreadGateTypeRole) {
    } else if (role == ThreadGateRulesRole) {
        AppBskyFeedThreadgate::Main record =
                LexiconsTypeUnknown::fromQVariant<AppBskyFeedThreadgate::Main>(
                        post.threadgate.record);
        QStringList rules = value.toStringList();
        record.allow_MentionRule.clear();
        record.allow_FollowingRule.clear();
        record.allow_ListRule.clear();
        for (const auto &rule : qAsConst(rules)) {
            if (rule == "mentioned") {
                record.allow_MentionRule.append(AppBskyFeedThreadgate::MentionRule());
            } else if (rule == "followed") {
                record.allow_FollowingRule.append(AppBskyFeedThreadgate::FollowingRule());
            } else if (rule.startsWith("at://")) {
                AppBskyFeedThreadgate::ListRule list;
                list.list = rule;
                record.allow_ListRule.append(list);
            }
        }
        post.threadgate.record.setValue(record);
    }
}

bool AtpAbstractListModel::hideByMutedWords(const QString &cid, const QString &author_did) const
{
    if (author_did == account().did)
        return false;

    return m_mutedPosts.contains(cid);
}

bool AtpAbstractListModel::cachePostsContainingMutedWords(
        const QString &cid, const AtProtocolType::AppBskyFeedPost::Main &record)
{
    if (cid.isEmpty())
        return false;

    QStringList targets = LexiconsTypeUnknown::copyTagsFromFacets(record.facets);
    targets.append(record.tags);

    bool contains = containsMutedWords(
            record.text, targets, LexiconsTypeUnknown::checkPartialMatchLanguage(record.langs));
    if (contains) {
        qDebug() << "Contains muted words" << cid << record.text << targets << record.langs;
        m_mutedPosts[cid] = cid;
    } else {
        if (m_mutedPosts.contains(cid)) {
            m_mutedPosts.remove(cid);
        }
        return contains;
    }
    return contains;
}

void AtpAbstractListModel::appendExtendMediaFileToClue(const QString &did, const QString &cid,
                                                       const QString &parent_cid)
{
    if (did.isEmpty() || cid.isEmpty() || parent_cid.isEmpty()) {
        return;
    } else {
        m_cueExtendMedia.append(BlobCueItem(did, cid, parent_cid));
    }
}

void AtpAbstractListModel::getExtendMediaFiles()
{
    BlobCueItem blob_item;
    if (m_cueExtendMedia.isEmpty()) {
        return;
    } else {
        blob_item = m_cueExtendMedia.front();
        m_cueExtendMedia.pop_front();
    }

    ComAtprotoSyncGetBlob *blob = new ComAtprotoSyncGetBlob(this);
    connect(blob, &ComAtprotoSyncGetBlob::finished, [=](bool success) {
        if (success) {
            QString path = saveMediaFile(blob->blobData(), blob_item.cid, blob->extension());
            if (!path.isEmpty()) {
                updateExtendMediaFile(blob_item.parent_cid);
            }
        }
        QTimer::singleShot(10, this, &AtpAbstractListModel::getExtendMediaFiles);
        blob->deleteLater();
    });
    blob->setAccount(account());
    blob->getBlob(blob_item.did, blob_item.cid);
}

QString AtpAbstractListModel::getMediaFilePath(const QString &cid, const QString &ext) const
{
    QString folder = Common::appTempFolder("ext_images");
    return QString("%1/%2.%3").arg(folder, cid, ext);
}

QString AtpAbstractListModel::saveMediaFile(const QByteArray &data, const QString &cid,
                                            const QString &ext)
{
    QString path = getMediaFilePath(cid, ext);
    QFile out(path);
    if (out.open(QFile::WriteOnly)) {
        qDebug().noquote().nospace() << "Save extend media file : " << path;
        out.write(data);
        out.close();
        return path;
    }
    return QString();
}

void AtpAbstractListModel::updateExtendMediaFile(const QString &parent_cid)
{
    Q_UNUSED(parent_cid)
}

bool AtpAbstractListModel::hasPinnedPost() const
{
    return false;
}

void AtpAbstractListModel::removePinnedPost() { }

bool AtpAbstractListModel::isPinnedPost(const QString &cid) const
{
    return (displayPinnedPost() && !m_currentPinnedPost.isEmpty() && cid == m_currentPinnedPost);
}

// 画像URLを取得する
// recordのembed/blobにgifがある場合はローカルに保存されているファイルパスを返す
// 無ければ通常のサムネ画像のURLを返す
QStringList AtpAbstractListModel::copyImagesFromPostView(
        const AtProtocolType::AppBskyFeedDefs::PostView &post,
        const AtProtocolType::LexiconsTypeUnknown::CopyImageType type) const
{
    QStringList ret = LexiconsTypeUnknown::copyImagesFromPostView(post, type);

    const AppBskyFeedPost::Main record =
            LexiconsTypeUnknown::fromQVariant<AppBskyFeedPost::Main>(post.record);
    for (int i = 0; i < record.embed_AppBskyEmbedImages_Main.images.length(); i++) {
        const auto &image = record.embed_AppBskyEmbedImages_Main.images.at(i);
        if (image.image.mimeType != "image/gif")
            continue;
        QString path = getMediaFilePath(image.image.cid,
                                        Common::mimeTypeToExtension(image.image.mimeType));
        if (QFile::exists(path)) {
            path = QUrl::fromLocalFile(path).toString();
            if (i < ret.length()) {
                ret[i] = path;
            } else {
                ret.append(path);
            }
        }
    }
    for (int i = 0; i < record.embed_AppBskyEmbedRecordWithMedia_Main.media_AppBskyEmbedImages_Main
                                .images.length();
         i++) {
        const auto &image = record.embed_AppBskyEmbedRecordWithMedia_Main
                                    .media_AppBskyEmbedImages_Main.images.at(i);
        if (image.image.mimeType != "image/gif")
            continue;
        QString path = getMediaFilePath(image.image.cid,
                                        Common::mimeTypeToExtension(image.image.mimeType));
        if (QFile::exists(path)) {
            path = QUrl::fromLocalFile(path).toString();
            if (i < ret.length()) {
                ret[i] = path;
            } else {
                ret.append(path);
            }
        }
    }
    return ret;
}

// recordのembed/blobにgifがあればgetBlobするためのキューに入れる
void AtpAbstractListModel::copyImagesFromPostViewToCue(
        const AtProtocolType::AppBskyFeedDefs::PostView &post)
{
    const AppBskyFeedPost::Main record =
            LexiconsTypeUnknown::fromQVariant<AppBskyFeedPost::Main>(post.record);
    for (int i = 0; i < record.embed_AppBskyEmbedImages_Main.images.length(); i++) {
        const auto &image = record.embed_AppBskyEmbedImages_Main.images.at(i);
        if (image.image.mimeType != "image/gif")
            continue;
        QString path = getMediaFilePath(image.image.cid,
                                        Common::mimeTypeToExtension(image.image.mimeType));
        if (!QFile::exists(path)) {
            appendExtendMediaFileToClue(post.author.did, image.image.cid, post.cid);
        }
    }
    for (int i = 0; i < record.embed_AppBskyEmbedRecordWithMedia_Main.media_AppBskyEmbedImages_Main
                                .images.length();
         i++) {
        const auto &image = record.embed_AppBskyEmbedRecordWithMedia_Main
                                    .media_AppBskyEmbedImages_Main.images.at(i);
        if (image.image.mimeType != "image/gif")
            continue;
        QString path = getMediaFilePath(image.image.cid,
                                        Common::mimeTypeToExtension(image.image.mimeType));
        if (!QFile::exists(path)) {
            appendExtendMediaFileToClue(post.author.did, image.image.cid, post.cid);
        }
    }
}

QString AtpAbstractListModel::atUriToOfficialUrl(const QString &uri, const QString &name) const
{
    QStringList items = uri.split("/");
    if (items.length() == 5) {
        return QString("https://bsky.app/profile/%1/%2/%3").arg(items.at(2), name, items.at(4));
    } else {
        return QString();
    }
}

QStringList AtpAbstractListModel::labelerDids() const
{
    return LabelerProvider::getInstance()->labelerDids(account());
}

ConfigurableLabelStatus AtpAbstractListModel::visibilityBylabeler(const QString &label,
                                                                  const bool for_image,
                                                                  const QString &labeler_did) const
{
    return LabelerProvider::getInstance()->visibility(account(), label, for_image, labeler_did);
}

bool AtpAbstractListModel::containsMutedWords(const QString &text, const QStringList &tags,
                                              const bool partial_match) const
{
    return LabelerProvider::getInstance()->containsMutedWords(account(), text, tags, partial_match);
}

QString AtpAbstractListModel::contentFilterMessage(const QString &label, const bool for_image,
                                                   const QString &labeler_did) const
{
    return LabelerProvider::getInstance()->message(account(), label, for_image, labeler_did);
}

QString AtpAbstractListModel::cursor() const
{
    return m_cursor;
}

void AtpAbstractListModel::setCursor(const QString &newCursor)
{
    m_cursor = newCursor;
}

int AtpAbstractListModel::displayInterval() const
{
    return m_displayInterval;
}

void AtpAbstractListModel::setDisplayInterval(int newDisplayInterval)
{
    if (m_displayInterval == newDisplayInterval)
        return;
    m_displayInterval = newDisplayInterval;
    emit displayIntervalChanged();
}

bool AtpAbstractListModel::autoLoading() const
{
    return m_timer.isActive();
}

void AtpAbstractListModel::setAutoLoading(bool newAutoLoading)
{
    if (newAutoLoading) {
        // Off -> On
        if (m_timer.isActive())
            return;
        m_timer.start(m_loadingInterval);
    } else {
        // * -> Off
        m_timer.stop();
    }
    emit autoLoadingChanged();
}

int AtpAbstractListModel::loadingInterval() const
{
    return m_loadingInterval;
}

void AtpAbstractListModel::setLoadingInterval(int newLoadingInterval)
{
    if (m_loadingInterval == newLoadingInterval)
        return;
    m_loadingInterval = newLoadingInterval;
    m_timer.setInterval(m_loadingInterval);
    emit loadingIntervalChanged();
}

QString AtpAbstractListModel::service() const
{
    return account().service;
}

QString AtpAbstractListModel::did() const
{
    return account().did;
}

QString AtpAbstractListModel::handle() const
{
    return account().handle;
}

QString AtpAbstractListModel::email() const
{
    return account().email;
}

QString AtpAbstractListModel::accessJwt() const
{
    return account().accessJwt;
}

QString AtpAbstractListModel::refreshJwt() const
{
    return account().refreshJwt;
}

bool AtpAbstractListModel::visibleContainingMutedWord() const
{
    return m_visibleContainingMutedWord;
}

void AtpAbstractListModel::setVisibleContainingMutedWord(bool newVisibleContainingMutedWord)
{
    if (m_visibleContainingMutedWord == newVisibleContainingMutedWord)
        return;
    m_visibleContainingMutedWord = newVisibleContainingMutedWord;
    emit visibleContainingMutedWordChanged();
}

QString AtpAbstractListModel::pinnedPost() const
{
    return m_pinnedPost;
}

void AtpAbstractListModel::setPinnedPost(const QString &newPinnedPost)
{
    if (m_pinnedPost == newPinnedPost)
        return;
    m_pinnedPost = newPinnedPost;
    emit pinnedPostChanged();

    qDebug() << "setPinnedPost" << m_pinnedPost;

    if (displayPinnedPost() && m_pinnedPost.isEmpty() && !running()) {
        removePinnedPost();
    }
}

bool AtpAbstractListModel::displayPinnedPost() const
{
    return m_displayPinnedPost;
}

void AtpAbstractListModel::setDisplayPinnedPost(bool newDisplayPinnedPost)
{
    if (m_displayPinnedPost == newDisplayPinnedPost)
        return;
    m_displayPinnedPost = newDisplayPinnedPost;
    emit displayPinnedPostChanged();
}
