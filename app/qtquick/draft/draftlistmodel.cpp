#include "draftlistmodel.h"

#include "atprotocol/app/bsky/draft/appbskydraftgetdrafts.h"
#include "atprotocol/lexicons_func_unknown.h"
#include "tools/deviceinfo.h"

using AtProtocolInterface::AppBskyDraftGetDrafts;
using namespace AtProtocolType;

DraftListModel::DraftListModel(QObject *parent)
    : AtpAbstractListModel { parent }, m_pageSize(25) { }

int DraftListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_drafts.count();
}

QVariant DraftListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    return item(index.row(), static_cast<DraftListModelRoles>(role));
}

QVariant DraftListModel::item(int row, DraftListModelRoles role) const
{
    if (row < 0 || row >= m_drafts.count())
        return QVariant();

    const auto &view = m_drafts.at(row);
    switch (role) {
    case DraftListModelRoles::ModelData:
        return QVariant();
    case DraftListModelRoles::IdRole:
        return view.id;
    case DraftListModelRoles::CreatedAtRole:
        return LexiconsTypeUnknown::formatDateTime(view.createdAt, true);
    case DraftListModelRoles::UpdatedAtRole:
        return LexiconsTypeUnknown::formatDateTime(view.updatedAt, true);
    case DraftListModelRoles::PrimaryTextRole:
        if (!view.draft.posts.isEmpty()) {
            return view.draft.posts.constFirst().text;
        }
        return QString();
    case DraftListModelRoles::PrimaryLabelsRole:
        if (!view.draft.posts.isEmpty()) {
            const auto &post = view.draft.posts.constFirst();
            if (post.labels_type
                == AppBskyDraftDefs::DraftPostLabelsType::labels_ComAtprotoLabelDefs_SelfLabels) {
                return labelsToStringList(post.labels_ComAtprotoLabelDefs_SelfLabels);
            }
        }
        return QVariant();
    case DraftListModelRoles::PrimaryEmbedImagesPathsRole:
        if (!view.draft.posts.isEmpty()) {
            const auto &post = view.draft.posts.constFirst();
            if (!post.embedImages.isEmpty()) {
                return imagesToPathsList(post.embedImages);
            }
        }
        return QVariant();
    case DraftListModelRoles::PrimaryEmbedImagesAltsRole:
        if (!view.draft.posts.isEmpty()) {
            const auto &post = view.draft.posts.constFirst();
            if (!post.embedImages.isEmpty()) {
                return imagesToAltsList(post.embedImages);
            }
        }
        return QVariant();
    case DraftListModelRoles::PrimaryEmbedVideosPathsRole:
        if (!view.draft.posts.isEmpty()) {
            const auto &post = view.draft.posts.constFirst();
            if (!post.embedVideos.isEmpty()) {
                return videosToPathsList(post.embedVideos);
            }
        }
        return QVariant();
    case DraftListModelRoles::PrimaryEmbedVideosAltsRole:
        if (!view.draft.posts.isEmpty()) {
            const auto &post = view.draft.posts.constFirst();
            if (!post.embedVideos.isEmpty()) {
                return videosToAltsList(post.embedVideos);
            }
        }
        return QVariant();
    case DraftListModelRoles::PrimaryEmbedVideosCaptionsRole:
        if (!view.draft.posts.isEmpty()) {
            const auto &post = view.draft.posts.constFirst();
            if (!post.embedVideos.isEmpty()) {
                return videosCaptionsToVariant(post.embedVideos);
            }
        }
        return QVariant();
    case DraftListModelRoles::PrimaryEmbedExternalsRole:
        if (!view.draft.posts.isEmpty()) {
            const auto &post = view.draft.posts.constFirst();
            if (!post.embedExternals.isEmpty()) {
                return externalsToUrisList(post.embedExternals);
            }
        }
        return QVariant();
    case DraftListModelRoles::PrimaryEmbedRecordsUrisRole:
        if (!view.draft.posts.isEmpty()) {
            const auto &post = view.draft.posts.constFirst();
            if (!post.embedRecords.isEmpty()) {
                return recordsToUrisList(post.embedRecords);
            }
        }
        return QVariant();
    case DraftListModelRoles::PrimaryEmbedRecordsCidsRole:
        if (!view.draft.posts.isEmpty()) {
            const auto &post = view.draft.posts.constFirst();
            if (!post.embedRecords.isEmpty()) {
                return recordsToCidsList(post.embedRecords);
            }
        }
        return QVariant();
    case DraftListModelRoles::LangsRole:
        return view.draft.langs;
    case DraftListModelRoles::PostgateEmbeddingRulesRole:
        return quoteEnabledFromDraft(view.draft);
    case DraftListModelRoles::ThreadGateTypeRole:
        return threadgateTypeFromDraft(view.draft);
    case DraftListModelRoles::ThreadGateRulesRole:
        return threadgateRulesFromDraft(view.draft);
    case DraftListModelRoles::PostCountRole:
        return view.draft.posts.count();
    case DraftListModelRoles::IsThreadRole:
        return view.draft.posts.count() > 1;
    case DraftListModelRoles::IsCurrentDeviceRole: {
        // Check if both deviceId and deviceName match the current device
        const QString currentDeviceId = DeviceInfo::getDeviceId();
        const QString currentDeviceName = DeviceInfo::getDeviceName();

        bool deviceIdMatches = false;
        bool deviceNameMatches = false;

        // Check if deviceId matches (both must be non-empty)
        if (!currentDeviceId.isEmpty() && !view.draft.deviceId.isEmpty()) {
            deviceIdMatches = (currentDeviceId == view.draft.deviceId);
        }

        // Check if deviceName matches (both must be non-empty)
        if (!currentDeviceName.isEmpty() && !view.draft.deviceName.isEmpty()) {
            deviceNameMatches = (currentDeviceName == view.draft.deviceName);
        }

        // Return true only if both match
        return deviceIdMatches && deviceNameMatches;
    }
    case DraftListModelRoles::DeviceNameRole:
        return view.draft.deviceName;
    default:
        break;
    }

    return QVariant();
}

bool DraftListModel::getLatest()
{
#ifdef HAGOROMO_LAYOUT_TEST
    addDummyDataForLayoutTest();
    return true;
#else
    if (running())
        return false;

    if (account().uuid.isEmpty())
        return false;

    setRunning(true);
    requestDrafts(QString(), false);
    return true;
#endif
}

bool DraftListModel::getNext()
{
    if (running())
        return false;

    if (cursor().isEmpty())
        return false;

    setRunning(true);
    requestDrafts(cursor(), true);
    return true;
}

void DraftListModel::clear()
{
    bool hadMore = hasMore();
    AtpAbstractListModel::clear();
    if (!m_drafts.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, m_drafts.count() - 1);
        m_drafts.clear();
        endRemoveRows();
    }
    if (hadMore != hasMore()) {
        emit hasMoreChanged();
    }
}

int DraftListModel::pageSize() const
{
    return m_pageSize;
}

void DraftListModel::setPageSize(int newPageSize)
{
    if (newPageSize <= 0 || newPageSize == m_pageSize)
        return;

    m_pageSize = newPageSize;
    emit pageSizeChanged();
}

bool DraftListModel::hasMore() const
{
    return !cursor().isEmpty();
}

int DraftListModel::indexOf(const QString &cid) const
{
    for (int i = 0; i < m_drafts.count(); ++i) {
        if (m_drafts.at(i).id == cid)
            return i;
    }
    return -1;
}

QString DraftListModel::getRecordText(const QString &cid)
{
    int row = indexOf(cid);
    if (row < 0)
        return QString();

    if (m_drafts.at(row).draft.posts.isEmpty())
        return QString();

    return m_drafts.at(row).draft.posts.constFirst().text;
}

QString DraftListModel::getOfficialUrl() const
{
    return QString();
}

QString DraftListModel::getItemOfficialUrl(int row) const
{
    Q_UNUSED(row)
    return QString();
}

QHash<int, QByteArray> DraftListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[DraftListModelRoles::ModelData] = "modelData";
    roles[DraftListModelRoles::IdRole] = "id";
    roles[DraftListModelRoles::CreatedAtRole] = "createdAt";
    roles[DraftListModelRoles::UpdatedAtRole] = "updatedAt";
    roles[DraftListModelRoles::PrimaryTextRole] = "primaryText";
    roles[DraftListModelRoles::PrimaryLabelsRole] = "primaryLabels";
    roles[DraftListModelRoles::PrimaryEmbedImagesPathsRole] = "primaryEmbedImagesPaths";
    roles[DraftListModelRoles::PrimaryEmbedImagesAltsRole] = "primaryEmbedImagesAlts";
    roles[DraftListModelRoles::PrimaryEmbedVideosPathsRole] = "primaryEmbedVideosPaths";
    roles[DraftListModelRoles::PrimaryEmbedVideosAltsRole] = "primaryEmbedVideosAlts";
    roles[DraftListModelRoles::PrimaryEmbedVideosCaptionsRole] = "primaryEmbedVideosCaptions";
    roles[DraftListModelRoles::PrimaryEmbedExternalsRole] = "primaryEmbedExternals";
    roles[DraftListModelRoles::PrimaryEmbedRecordsUrisRole] = "primaryEmbedRecordsUris";
    roles[DraftListModelRoles::PrimaryEmbedRecordsCidsRole] = "primaryEmbedRecordsCids";
    roles[DraftListModelRoles::LangsRole] = "langs";
    roles[DraftListModelRoles::PostgateEmbeddingRulesRole] = "postgateEmbeddingRules";
    roles[DraftListModelRoles::ThreadGateTypeRole] = "threadGateType";
    roles[DraftListModelRoles::ThreadGateRulesRole] = "threadGateRules";
    roles[DraftListModelRoles::PostCountRole] = "postCount";
    roles[DraftListModelRoles::IsThreadRole] = "isThread";
    roles[DraftListModelRoles::IsCurrentDeviceRole] = "isCurrentDevice";
    roles[DraftListModelRoles::DeviceNameRole] = "deviceName";
    return roles;
}

bool DraftListModel::aggregateQueuedPosts(const QString &cid, const bool next)
{
    Q_UNUSED(cid)
    Q_UNUSED(next)
    return true;
}

bool DraftListModel::aggregated(const QString &cid) const
{
    Q_UNUSED(cid)
    return false;
}

void DraftListModel::finishedDisplayingQueuedPosts() { }

bool DraftListModel::checkVisibility(const QString &cid)
{
    Q_UNUSED(cid)
    return true;
}

void DraftListModel::requestDrafts(const QString &cursorValue, bool append)
{
    AppBskyDraftGetDrafts *get = new AppBskyDraftGetDrafts(this);
    connect(get, &AppBskyDraftGetDrafts::finished, this, [=](bool success) {
        if (success) {
            const auto drafts = get->draftsList();
            if (!append && !m_drafts.isEmpty()) {
                beginRemoveRows(QModelIndex(), 0, m_drafts.count() - 1);
                m_drafts.clear();
                endRemoveRows();
            }
            if (!drafts.isEmpty()) {
                int insertPos = m_drafts.count();
                beginInsertRows(QModelIndex(), insertPos, insertPos + drafts.count() - 1);
                for (const auto &draft : drafts) {
                    m_drafts.append(draft);
                }
                endInsertRows();
            }

            bool hadMore = hasMore();
            setCursor(get->cursor());
            if (hadMore != hasMore()) {
                emit hasMoreChanged();
            }
        } else {
            emit errorOccured(get->errorCode(), get->errorMessage());
        }
        setRunning(false);
        get->deleteLater();
    });
    get->setAccount(account());
    get->getDrafts(m_pageSize, cursorValue);
}

QStringList DraftListModel::labelsToStringList(
        const AtProtocolType::ComAtprotoLabelDefs::SelfLabels &labels) const
{
    QStringList list;
    for (const auto &value : labels.values) {
        list.append(value.val);
    }
    return list;
}

QStringList DraftListModel::imagesToPathsList(
        const QList<AtProtocolType::AppBskyDraftDefs::DraftEmbedImage> &images) const
{
    QStringList list;
    for (const auto &image : images) {
        list.append(image.localRef.path);
    }
    return list;
}

QStringList DraftListModel::imagesToAltsList(
        const QList<AtProtocolType::AppBskyDraftDefs::DraftEmbedImage> &images) const
{
    QStringList list;
    for (const auto &image : images) {
        list.append(image.alt);
    }
    return list;
}

QStringList DraftListModel::videosToPathsList(
        const QList<AtProtocolType::AppBskyDraftDefs::DraftEmbedVideo> &videos) const
{
    QStringList list;
    for (const auto &video : videos) {
        list.append(video.localRef.path);
    }
    return list;
}

QStringList DraftListModel::videosToAltsList(
        const QList<AtProtocolType::AppBskyDraftDefs::DraftEmbedVideo> &videos) const
{
    QStringList list;
    for (const auto &video : videos) {
        list.append(video.alt);
    }
    return list;
}

QVariantList DraftListModel::videosCaptionsToVariant(
        const QList<AtProtocolType::AppBskyDraftDefs::DraftEmbedVideo> &videos) const
{
    QVariantList list;
    for (const auto &video : videos) {
        if (!video.captions.isEmpty()) {
            QVariantList captions;
            for (const auto &caption : video.captions) {
                QVariantMap map;
                map["lang"] = caption.lang;
                map["content"] = caption.content;
                captions.append(map);
            }
            list.append(captions);
        } else {
            list.append(QVariantList());
        }
    }
    return list;
}

QStringList DraftListModel::externalsToUrisList(
        const QList<AtProtocolType::AppBskyDraftDefs::DraftEmbedExternal> &externals) const
{
    QStringList list;
    for (const auto &external : externals) {
        list.append(external.uri);
    }
    return list;
}

QStringList DraftListModel::recordsToUrisList(
        const QList<AtProtocolType::AppBskyDraftDefs::DraftEmbedRecord> &records) const
{
    QStringList list;
    for (const auto &record : records) {
        list.append(record.record.uri);
    }
    return list;
}

QStringList DraftListModel::recordsToCidsList(
        const QList<AtProtocolType::AppBskyDraftDefs::DraftEmbedRecord> &records) const
{
    QStringList list;
    for (const auto &record : records) {
        list.append(record.record.cid);
    }
    return list;
}

bool DraftListModel::quoteEnabledFromDraft(
        const AtProtocolType::AppBskyDraftDefs::Draft &draft) const
{
    // If disableRule exists, quote is disabled (return false)
    // Otherwise, quote is enabled (return true)
    if (draft.postgateEmbeddingRules_type
        == AppBskyDraftDefs::DraftPostgateEmbeddingRulesType::
                postgateEmbeddingRules_AppBskyFeedPostgate_DisableRule) {
        if (!draft.postgateEmbeddingRules_AppBskyFeedPostgate_DisableRule.isEmpty()) {
            return false;
        }
    }
    return true;
}

QString
DraftListModel::threadgateTypeFromDraft(const AtProtocolType::AppBskyDraftDefs::Draft &draft) const
{
    if (draft.threadgateAllow_type == AppBskyDraftDefs::DraftThreadgateAllowType::none) {
        return QStringLiteral("everybody");
    }

    // Check if all lists are empty (nobody)
    bool hasRules = false;
    if (!draft.threadgateAllow_AppBskyFeedThreadgate_MentionRule.isEmpty()) {
        hasRules = true;
    }
    if (!draft.threadgateAllow_AppBskyFeedThreadgate_FollowerRule.isEmpty()) {
        hasRules = true;
    }
    if (!draft.threadgateAllow_AppBskyFeedThreadgate_FollowingRule.isEmpty()) {
        hasRules = true;
    }
    if (!draft.threadgateAllow_AppBskyFeedThreadgate_ListRule.isEmpty()) {
        hasRules = true;
    }

    if (!hasRules) {
        return QStringLiteral("nobody");
    }

    return QStringLiteral("choice");
}

QStringList
DraftListModel::threadgateRulesFromDraft(const AtProtocolType::AppBskyDraftDefs::Draft &draft) const
{
    QStringList rules;

    // Check all rule lists since the parser may put data in any of them
    for (int i = 0; i < draft.threadgateAllow_AppBskyFeedThreadgate_MentionRule.count(); ++i) {
        rules.append(QStringLiteral("mentioned"));
    }
    for (int i = 0; i < draft.threadgateAllow_AppBskyFeedThreadgate_FollowerRule.count(); ++i) {
        rules.append(QStringLiteral("follower"));
    }
    for (int i = 0; i < draft.threadgateAllow_AppBskyFeedThreadgate_FollowingRule.count(); ++i) {
        rules.append(QStringLiteral("followed"));
    }
    for (const auto &rule : draft.threadgateAllow_AppBskyFeedThreadgate_ListRule) {
        rules.append(rule.list);
    }

    return rules;
}

#ifdef HAGOROMO_LAYOUT_TEST
void DraftListModel::addDummyDataForLayoutTest()
{
    using namespace AtProtocolType;

    // Dummy data 1: Simple text post
    {
        AppBskyDraftDefs::DraftView draftView;
        draftView.id = "3l6k5j4h3g2f1a";
        draftView.createdAt = "2026-02-20T09:30:00.000Z";
        draftView.updatedAt = "2026-02-20T15:45:00.000Z";
        draftView.draft.deviceId = "test-device-id-001";
        draftView.draft.deviceName = "Test Device";

        AppBskyDraftDefs::DraftPost post;
        post.text = "This is a simple test draft post for layout testing. It contains basic text.";
        draftView.draft.posts.append(post);
        draftView.draft.langs.append("en");

        beginInsertRows(QModelIndex(), m_drafts.count(), m_drafts.count());
        m_drafts.append(draftView);
        endInsertRows();
    }

    // Dummy data 2: Thread with multiple posts
    {
        AppBskyDraftDefs::DraftView draftView;
        draftView.id = "3l6k5j4h3g2f2b";
        draftView.createdAt = "2026-02-19T14:20:00.000Z";
        draftView.updatedAt = "2026-02-20T10:30:00.000Z";
        draftView.draft.deviceId = "test-device-id-001";
        draftView.draft.deviceName = "Test Device";

        AppBskyDraftDefs::DraftPost post1;
        post1.text = "This is the first post in a thread. 🧵";
        draftView.draft.posts.append(post1);

        AppBskyDraftDefs::DraftPost post2;
        post2.text = "This is the second post continuing the thread with more information...";
        draftView.draft.posts.append(post2);

        AppBskyDraftDefs::DraftPost post3;
        post3.text = "And this is the final post in the thread!";
        draftView.draft.posts.append(post3);

        draftView.draft.langs.append("en");

        beginInsertRows(QModelIndex(), m_drafts.count(), m_drafts.count());
        m_drafts.append(draftView);
        endInsertRows();
    }

    // Dummy data 3: Post with images
    {
        AppBskyDraftDefs::DraftView draftView;
        draftView.id = "3l6k5j4h3g2f3c";
        draftView.createdAt = "2026-02-18T08:15:00.000Z";
        draftView.updatedAt = "2026-02-18T08:30:00.000Z";
        draftView.draft.deviceId = "test-device-id-002";
        draftView.draft.deviceName = "Test Tablet";

        AppBskyDraftDefs::DraftPost post;
        post.text = "Check out these amazing photos! 📸";

        AppBskyDraftDefs::DraftEmbedImage image1;
        image1.localRef.path = "/path/to/test/image1.jpg";
        image1.alt = "First test image";
        post.embedImages.append(image1);

        AppBskyDraftDefs::DraftEmbedImage image2;
        image2.localRef.path = "/path/to/test/image2.jpg";
        image2.alt = "Second test image";
        post.embedImages.append(image2);

        draftView.draft.posts.append(post);
        draftView.draft.langs.append("en");

        beginInsertRows(QModelIndex(), m_drafts.count(), m_drafts.count());
        m_drafts.append(draftView);
        endInsertRows();
    }

    // Dummy data 4: Long text post
    {
        AppBskyDraftDefs::DraftView draftView;
        draftView.id = "3l6k5j4h3g2f4d";
        draftView.createdAt = "2026-02-21T06:00:00.000Z";
        draftView.updatedAt = "2026-02-21T07:15:00.000Z";
        draftView.draft.deviceId = "test-device-id-001";
        draftView.draft.deviceName = "Test Device";

        AppBskyDraftDefs::DraftPost post;
        post.text =
                "Looking up at the eaves where droplets fall regularly."
                "The sky beyond is covered in dark, heavy clouds, and the falling rain shows no "
                "sign of stopping."
                "Avoiding that gloomy sky, I turn my gaze to the dog sitting proudly at my feet."
                "Are you the heir to this shop?";
        draftView.draft.posts.append(post);
        draftView.draft.langs.append("en");

        beginInsertRows(QModelIndex(), m_drafts.count(), m_drafts.count());
        m_drafts.append(draftView);
        endInsertRows();
    }

    // Dummy data 5: Post with content warning labels
    {
        AppBskyDraftDefs::DraftView draftView;
        draftView.id = "3l6k5j4h3g2f5e";
        draftView.createdAt = "2026-02-17T12:00:00.000Z";
        draftView.updatedAt = "2026-02-17T12:30:00.000Z";
        draftView.draft.deviceId = "test-device-id-003";
        draftView.draft.deviceName = "Test Desktop";

        AppBskyDraftDefs::DraftPost post;
        post.text = "This post has content warnings attached. Testing label display in the layout.";
        post.labels_type =
                AppBskyDraftDefs::DraftPostLabelsType::labels_ComAtprotoLabelDefs_SelfLabels;

        ComAtprotoLabelDefs::SelfLabel label1;
        label1.val = "graphic-media";
        post.labels_ComAtprotoLabelDefs_SelfLabels.values.append(label1);

        ComAtprotoLabelDefs::SelfLabel label2;
        label2.val = "nudity";
        post.labels_ComAtprotoLabelDefs_SelfLabels.values.append(label2);

        draftView.draft.posts.append(post);
        draftView.draft.langs.append("en");

        beginInsertRows(QModelIndex(), m_drafts.count(), m_drafts.count());
        m_drafts.append(draftView);
        endInsertRows();
    }

    // Dummy data 6: Post with external link
    {
        AppBskyDraftDefs::DraftView draftView;
        draftView.id = "3l6k5j4h3g2f6f";
        draftView.createdAt = "2026-02-16T16:45:00.000Z";
        draftView.updatedAt = "2026-02-16T17:00:00.000Z";
        draftView.draft.deviceId = "test-device-id-001";
        draftView.draft.deviceName = "Test Device";

        AppBskyDraftDefs::DraftPost post;
        post.text = "Sharing an interesting article 🔗";

        AppBskyDraftDefs::DraftEmbedExternal external;
        external.uri = "https://example.com/interesting-article";
        post.embedExternals.append(external);

        draftView.draft.posts.append(post);
        draftView.draft.langs.append("en");

        beginInsertRows(QModelIndex(), m_drafts.count(), m_drafts.count());
        m_drafts.append(draftView);
        endInsertRows();
    }

    // Dummy data 7: Japanese text post
    {
        AppBskyDraftDefs::DraftView draftView;
        draftView.id = "3l6k5j4h3g2f7g";
        draftView.createdAt = "2026-02-15T10:00:00.000Z";
        draftView.updatedAt = "2026-02-15T10:30:00.000Z";
        draftView.draft.deviceId = "test-device-id-001";
        draftView.draft.deviceName = "Test Device";

        AppBskyDraftDefs::DraftPost post;
        post.text = "これはレイアウトテスト用の日本語のドラフト投稿です。様々な文字の表示を確認しま"
                    "す。";
        draftView.draft.posts.append(post);
        draftView.draft.langs.append("ja");

        beginInsertRows(QModelIndex(), m_drafts.count(), m_drafts.count());
        m_drafts.append(draftView);
        endInsertRows();
    }

    // Dummy data 8: Empty draft (edge case)
    {
        AppBskyDraftDefs::DraftView draftView;
        draftView.id = "3l6k5j4h3g2f8h";
        draftView.createdAt = "2026-02-14T20:00:00.000Z";
        draftView.updatedAt = "2026-02-14T20:00:00.000Z";
        draftView.draft.deviceId = "test-device-id-001";
        draftView.draft.deviceName = "Test Device";

        AppBskyDraftDefs::DraftPost post;
        post.text = "";
        draftView.draft.posts.append(post);

        beginInsertRows(QModelIndex(), m_drafts.count(), m_drafts.count());
        m_drafts.append(draftView);
        endInsertRows();
    }

    // Dummy data 9: Post with video
    {
        AppBskyDraftDefs::DraftView draftView;
        draftView.id = "3l6k5j4h3g2f9i";
        draftView.createdAt = "2026-02-13T11:30:00.000Z";
        draftView.updatedAt = "2026-02-13T12:00:00.000Z";
        draftView.draft.deviceId = "test-device-id-002";
        draftView.draft.deviceName = "Test Tablet";

        AppBskyDraftDefs::DraftPost post;
        post.text = "Check out this video! 🎥";

        AppBskyDraftDefs::DraftEmbedVideo video;
        video.localRef.path = "/path/to/test/video.mp4";
        video.alt = "Test video description";

        AppBskyDraftDefs::DraftEmbedCaption caption;
        caption.lang = "en";
        caption.content = "English caption for the video";
        video.captions.append(caption);

        post.embedVideos.append(video);
        draftView.draft.posts.append(post);
        draftView.draft.langs.append("en");

        beginInsertRows(QModelIndex(), m_drafts.count(), m_drafts.count());
        m_drafts.append(draftView);
        endInsertRows();
    }

    // Dummy data 10: Post with quote/record
    {
        AppBskyDraftDefs::DraftView draftView;
        draftView.id = "3l6k5j4h3g2f0j";
        draftView.createdAt = "2026-02-12T14:20:00.000Z";
        draftView.updatedAt = "2026-02-12T14:45:00.000Z";
        draftView.draft.deviceId = "test-device-id-001";
        draftView.draft.deviceName = "Test Device";

        AppBskyDraftDefs::DraftPost post;
        post.text = "Quoting an interesting post";

        AppBskyDraftDefs::DraftEmbedRecord record;
        record.record.uri = "at://did:plc:test123/app.bsky.feed.post/3l6k5j4h3g2abc";
        record.record.cid = "bafyreicidtest123456789";
        post.embedRecords.append(record);

        draftView.draft.posts.append(post);
        draftView.draft.langs.append("en");

        beginInsertRows(QModelIndex(), m_drafts.count(), m_drafts.count());
        m_drafts.append(draftView);
        endInsertRows();
    }
}
#endif
