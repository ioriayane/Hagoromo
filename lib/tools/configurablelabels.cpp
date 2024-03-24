#include "configurablelabels.h"

#include "atprotocol/app/bsky/actor/appbskyactorgetpreferences.h"
#include "atprotocol/app/bsky/actor/appbskyactorputpreferences.h"
#include "atprotocol/app/bsky/labeler/appbskylabelergetservices.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QHash>

#define GLOBAL_LABELER_KEY QStringLiteral("__globally__")
#define BSKY_OFFICIAL_LABELER_DID QStringLiteral("did:plc:ar7c4by46qjdydhdevvrndac")

using AtProtocolInterface::AppBskyActorGetPreferences;
using AtProtocolInterface::AppBskyActorPutPreferences;
using AtProtocolInterface::AppBskyLabelerGetServices;
using namespace AtProtocolType;

ConfigurableLabels::ConfigurableLabels(QObject *parent)
    : AtProtocolInterface::AtProtocolAccount { parent },
      m_refreshLabelers(false),
      m_enableAdultContent(true),
      m_running(false)
{
    m_regSpace.setPattern("\\s");
    initializeLabels();
}

ConfigurableLabels &ConfigurableLabels::operator=(ConfigurableLabels &other)
{
    m_enableAdultContent = other.m_enableAdultContent;
    m_labels.clear();
    QMapIterator<QString, QList<ConfigurableLabelItem>> labels_i(other.m_labels);
    while (labels_i.hasNext()) {
        labels_i.next();
        m_labels[labels_i.key()].clear();
        for (const auto &label : labels_i.value()) {
            m_labels[labels_i.key()].append(label);
        }
    }
    m_targetLabelerDids = other.m_targetLabelerDids;
    m_labelers.clear();
    QMapIterator<QString, LabelerItem> labelers_i(other.m_labelers);
    while (labelers_i.hasNext()) {
        labelers_i.next();
        m_labelers[labelers_i.key()] = labelers_i.value();
    }

    m_mutedWords.clear();
    m_mutedWordsHash.clear();
    m_mutedWordsTagHash.clear();
    for (const auto &word : qAsConst(other.m_mutedWords)) {
        m_mutedWords.append(word);
        m_mutedWordsHash[word.value] = word;
        m_mutedWordsTagHash[removeSharp(word.value)] = word;
    }

    return *this;
}

int ConfigurableLabels::count(const QString &labeler_did) const
{
    QString key = labeler_did.isEmpty() ? GLOBAL_LABELER_KEY : labeler_did;
    if (!m_labels.contains(key))
        return 0;
    return m_labels.value(key).count();
}

bool ConfigurableLabels::load()
{
    if (running())
        return false;
    setRunning(true);

    clearMutedWord();
    initializeLabels();

    AppBskyActorGetPreferences *pref = new AppBskyActorGetPreferences(this);
    connect(pref, &AppBskyActorGetPreferences::finished, [=](bool success) {
        if (success) {
            m_targetLabelerDids.clear();
            for (const auto &labelers_pref : *pref->labelersPrefList()) {
                for (const auto &labeler : labelers_pref.labelers) {
                    m_targetLabelerDids.append(labeler.did);
                }
            }
            if (!m_targetLabelerDids.contains(BSKY_OFFICIAL_LABELER_DID)) {
                m_targetLabelerDids.insert(0, BSKY_OFFICIAL_LABELER_DID);
            }
            loadLabelers(m_targetLabelerDids, [=](bool load_labelers_success) {
                if (load_labelers_success) {
                    m_enableAdultContent = pref->adultContentPref().enabled;
                    for (const auto &item : *pref->contentLabelPrefList()) {
                        int index = indexOf(item.label, item.labelerDid);
                        ConfigurableLabelStatus status = toLabelStatus(item.visibility);
                        if (index >= 0) {
                            // labelerDid.isEmptyのときもこっち（global
                            // labelや互換性用のlabelのとき）
                            setStatus(index, status, item.labelerDid);
                        } else if (item.labelerDid.isEmpty()) {
                            // labelerDid.isEmptyのときもこっち（未知のラベルの設定）
                            // 読み込まない
                        } else {
                            ConfigurableLabelItem label;
                            label.id = item.label;
                            label.labeler_did = item.labelerDid;
                            label.status = status;
                            label.values << item.label;
                            // app.bsky.labeler.getServicesで取得するデータ
                            label.title = item.label.toCaseFolded();
                            label.subtitle = label.title;
                            label.warning = label.title;
                            label.is_adult_imagery = false;
                            label.foldable_range = ConfigurableLabelFoldableRange::Content;
                            label.configurable = true;
                            m_labels[label.labeler_did].append(label);
                        }
                    }
                    int group = 0;
                    for (const auto &pref : *pref->mutedWordsPrefList()) {
                        for (const auto &item : pref.items) {
                            MutedWordItem mute;
                            mute.group = group;
                            mute.value = item.value;
                            if (item.targets.contains("content")) {
                                mute.targets.append(MutedWordTarget::Content);
                            }
                            if (item.targets.contains("tag")) {
                                mute.targets.append(MutedWordTarget::Tag);
                            }
                            m_mutedWords.append(mute);
                            m_mutedWordsHash[mute.value] = mute;
                            m_mutedWordsTagHash[removeSharp(mute.value)] = mute;
                        }
                        group++;
                    }
                }
                setRunning(false);
                emit finished(load_labelers_success);
                pref->deleteLater();
            });
        } else {
            qDebug() << "Fail load preferences for " << account().handle;
            setRunning(false);
            emit finished(success);
            pref->deleteLater();
        }
    });
    pref->setAccount(account());
    pref->getPreferences();
    return true;
}

void ConfigurableLabels::loadLabelers(const QStringList &dids, std::function<void(bool)> callback)
{
    if (!m_labelers.isEmpty() && !refreshLabelers()) {
        // すでに持っている場合はリフレッシュ指示がなければ空振りする
        qDebug() << "loadLabelers() : Already have labelers info.";
        callback(true);
        return;
    }

    QStringList t_dids = dids;
    if (!t_dids.contains(BSKY_OFFICIAL_LABELER_DID)) {
        t_dids.append(BSKY_OFFICIAL_LABELER_DID);
    }

    // グローバル以外をクリア
    for (const auto &key : m_labels.keys()) {
        if (key == GLOBAL_LABELER_KEY)
            continue;
        m_labels.remove(key);
    }
    for (const auto &key : m_labelers.keys()) {
        if (key == GLOBAL_LABELER_KEY)
            continue;
        m_labelers.remove(key);
    }

    AppBskyLabelerGetServices *services = new AppBskyLabelerGetServices(this);
    connect(services, &AppBskyLabelerGetServices::finished, [=](bool success) {
        if (success) {
            for (const auto &labeler : *services->labelerViewDetails()) {
                LabelerItem labeler_item;
                labeler_item.did = labeler.creator.did;
                labeler_item.handle = labeler.creator.handle;
                labeler_item.display_name = labeler.creator.displayName;
                labeler_item.description = labeler.creator.description;
                m_labelers[labeler.creator.did] = labeler_item;
                for (const auto &policy : labeler.policies.labelValueDefinitions) {
                    ComAtprotoLabelDefs::LabelValueDefinitionStrings label_value_def;
                    for (const auto &locale : policy.locales) {
                        if (QLocale(locale.lang).language() == QLocale::system().language()) {
                            label_value_def = locale;
                            break;
                        }
                    }
                    if (label_value_def.lang.isEmpty() && !policy.locales.isEmpty()) {
                        label_value_def = policy.locales.at(0);
                    }
                    ConfigurableLabelItem label_item;
                    label_item.labeler_did = labeler_item.did;
                    label_item.id = policy.identifier;
                    label_item.title = label_value_def.name;
                    label_item.subtitle = label_value_def.description;
                    label_item.warning = label_item.title;
                    label_item.values << policy.identifier;
                    label_item.is_adult_imagery = false;
                    label_item.foldable_range = toLabelFoldableRange(policy.blurs);
                    label_item.level = toLabelLevel(policy.severity);
                    label_item.default_status = toLabelStatus(policy.defaultSetting);
                    label_item.status = label_item.default_status;
                    m_labels[labeler_item.did].append(label_item);
                }
            }
        }
        callback(success);
        services->deleteLater();
    });
    services->setAccount(account());
    services->getServices(t_dids, true);
}

bool ConfigurableLabels::save()
{
    if (running())
        return false;
    setRunning(true);

    AppBskyActorGetPreferences *pref = new AppBskyActorGetPreferences(this);
    connect(pref, &AppBskyActorGetPreferences::finished, [=](bool success) {
        if (success) {
            if (!putPreferences(updatePreferencesJson(pref->replyJson()))) {
                setRunning(false);
            }
        } else {
            setRunning(false);
            emit finished(success);
        }
        pref->deleteLater();
    });
    pref->setAccount(account());
    pref->getPreferences();
    return true;
}

int ConfigurableLabels::indexOf(const QString &id, const QString &labeler_did) const
{
    if (id.isEmpty())
        return -1;

    QString key = labeler_did.startsWith("did:") ? labeler_did : GLOBAL_LABELER_KEY;
    int index = 0;
    for (const auto &label : m_labels[key]) {
        if (label.id == id)
            return index;
        index++;
    }
    return -1;
}

ConfigurableLabelStatus ConfigurableLabels::visibility(const QString &label, const bool for_image,
                                                       const QString &labeler_did) const
{
    QString key = labeler_did.isEmpty() ? GLOBAL_LABELER_KEY : labeler_did;

    ConfigurableLabelFoldableRange foldable_range = for_image
            ? ConfigurableLabelFoldableRange::Media
            : ConfigurableLabelFoldableRange::Content;
    ConfigurableLabelStatus result = ConfigurableLabelStatus::Unknown;
    if (m_labels.contains(key)) {
        for (const auto &item : m_labels.value(key)) {
            if (item.values.contains(label) && item.foldable_range == foldable_range) {
                // 画像用のラベルの検索か、それ以外か。どちらか一方のみ。つまり、一致しているときだけ。
                if (item.is_adult_imagery) {
                    if (m_enableAdultContent) {
                        result = item.status;
                    } else {
                        result = ConfigurableLabelStatus::Hide;
                    }
                } else {
                    result = item.status;
                }
                break;
            }
        }
    }
    if (result != ConfigurableLabelStatus::Unknown) {
        // 見つかった
        return result;
    } else if (labeler_did.isEmpty()) {
        // グローバルを検索しても見つからないときは表示
        return ConfigurableLabelStatus::Show;
    } else {
        // ラベラー指定で見つからないときはグローバルを検索
        return visibility(label, for_image, QString());
    }
}

QString ConfigurableLabels::message(const QString &label, const bool for_image,
                                    const QString &labeler_did) const
{
    QString key = labeler_did.isEmpty() ? GLOBAL_LABELER_KEY : labeler_did;

    ConfigurableLabelFoldableRange foldable_range = for_image
            ? ConfigurableLabelFoldableRange::Media
            : ConfigurableLabelFoldableRange::Content;
    QString result;
    if (m_labels.contains(key)) {
        for (const auto &item : m_labels.value(key)) {
            if (item.values.contains(label) && item.foldable_range == foldable_range) {
                result = item.warning;
                break;
            }
        }
    }
    if (!result.isEmpty()) {
        return result;
    } else if (labeler_did.isEmpty()) {
        return QString();
    } else {
        return message(label, for_image, QString());
    }
}

QString ConfigurableLabels::title(const int index, const QString &labeler_did) const
{
    QString key = labeler_did.isEmpty() ? GLOBAL_LABELER_KEY : labeler_did;
    if (!m_labels.contains(key))
        return QString();
    if (index < 0 || index >= m_labels.value(key).length())
        return QString();

    return m_labels.value(key).at(index).title;
}

QString ConfigurableLabels::title(const QString &label, const bool for_image,
                                  const QString &labeler_did) const
{
    QString key = labeler_did.isEmpty() ? GLOBAL_LABELER_KEY : labeler_did;
    if (!m_labels.contains(key))
        return QString();

    ConfigurableLabelFoldableRange foldable_range = for_image
            ? ConfigurableLabelFoldableRange::Media
            : ConfigurableLabelFoldableRange::Content;
    QString result;
    for (const auto &item : m_labels.value(key)) {
        if (item.values.contains(label) && item.foldable_range == foldable_range) {
            // 画像用のラベルの検索か、それ以外か。どちらか一方のみ。つまり、一致しているときだけ。
            if (item.is_adult_imagery) {
                if (m_enableAdultContent) {
                    result = item.title;
                } else {
                    result = QString();
                }
            } else {
                result = item.title;
            }
            break;
        }
    }
    return result;
}

QString ConfigurableLabels::description(const int index, const QString &labeler_did) const
{
    QString key = labeler_did.isEmpty() ? GLOBAL_LABELER_KEY : labeler_did;
    if (!m_labels.contains(key))
        return QString();
    if (index < 0 || index >= m_labels.value(key).length())
        return QString();
    return m_labels.value(key).at(index).subtitle;
}

ConfigurableLabelFoldableRange ConfigurableLabels::foldableRange(const int index,
                                                                 const QString &labeler_did) const
{
    QString key = labeler_did.isEmpty() ? GLOBAL_LABELER_KEY : labeler_did;
    if (!m_labels.contains(key))
        return ConfigurableLabelFoldableRange::None;
    if (index < 0 || index >= m_labels.value(key).length())
        return ConfigurableLabelFoldableRange::None;
    return m_labels.value(key).at(index).foldable_range;
}

ConfigurableLabelStatus ConfigurableLabels::status(const int index,
                                                   const QString &labeler_did) const
{
    QString key = labeler_did.isEmpty() ? GLOBAL_LABELER_KEY : labeler_did;
    if (!m_labels.contains(key))
        return ConfigurableLabelStatus::Show;
    if (index < 0 || index >= m_labels.value(key).length())
        return ConfigurableLabelStatus::Show;

    return m_labels.value(key).at(index).status;
}

void ConfigurableLabels::setStatus(const int index, const ConfigurableLabelStatus status,
                                   const QString &labeler_did)
{
    QString key = labeler_did.isEmpty() ? GLOBAL_LABELER_KEY : labeler_did;
    if (!m_labels.contains(key))
        return;
    if (index < 0 || index >= m_labels.value(key).length())
        return;
    if (m_labels.value(key).at(index).configurable == false)
        return;

    m_labels[key][index].status = status;
}

bool ConfigurableLabels::isAdultImagery(const int index, const QString &labeler_did) const
{
    QString key = labeler_did.isEmpty() ? GLOBAL_LABELER_KEY : labeler_did;
    if (!m_labels.contains(key))
        return false;
    if (index < 0 || index >= m_labels.value(key).length())
        return false;

    return m_labels.value(key).at(index).is_adult_imagery;
}

bool ConfigurableLabels::configurable(const int index, const QString &labeler_did) const
{
    QString key = labeler_did.isEmpty() ? GLOBAL_LABELER_KEY : labeler_did;
    if (!m_labels.contains(key))
        return false;
    if (index < 0 || index >= m_labels.value(key).length())
        return false;

    return m_labels.value(key).at(index).configurable;
}

int ConfigurableLabels::targetLabelerCount() const
{
    return m_targetLabelerDids.count();
}

QString ConfigurableLabels::targetLabelerDid(const int index) const
{
    if (index < 0 || index >= m_targetLabelerDids.count())
        return QString();
    return m_targetLabelerDids.at(index);
}

int ConfigurableLabels::labelerCount() const
{
    return m_labelers.count();
}

QStringList ConfigurableLabels::labelerDids() const
{
    return m_labelers.keys();
}

QString ConfigurableLabels::labelerHandle(const QString &did) const
{
    if (m_labelers.contains(did)) {
        return m_labelers.value(did).handle;
    } else {
        return QString();
    }
}

QString ConfigurableLabels::labelerDisplayName(const QString &did) const
{
    if (m_labelers.contains(did)) {
        return m_labelers.value(did).display_name;
    } else {
        return QString();
    }
}

QString ConfigurableLabels::labelerDescription(const QString &did) const
{
    if (m_labelers.contains(did)) {
        return m_labelers.value(did).description;
    } else {
        return QString();
    }
}

int ConfigurableLabels::mutedWordCount() const
{
    return m_mutedWords.count();
}

MutedWordItem ConfigurableLabels::getMutedWordItem(const int index) const
{
    if (index < 0 || index >= mutedWordCount())
        return MutedWordItem();

    return m_mutedWords.at(index);
}

void ConfigurableLabels::insertMutedWord(const int index, const QString &value,
                                         const QList<MutedWordTarget> &targets)
{
    if (index < 0 || value.isEmpty())
        return;
    MutedWordItem item;
    item.group = 0;
    item.value = value;
    item.targets = targets;
    if (index >= mutedWordCount()) {
        m_mutedWords.append(item);
    } else {
        m_mutedWords.insert(index, item);
    }
    m_mutedWordsHash[value] = item;
    QString key = removeSharp(value);
    m_mutedWordsTagHash[key] = item;
}

void ConfigurableLabels::updateMutedWord(const int index, const QString &value,
                                         const QList<MutedWordTarget> &targets)
{
    if (index < 0 || index >= mutedWordCount() || value.isEmpty())
        return;
    m_mutedWords[index].value = value;
    m_mutedWords[index].targets = targets;
    if (m_mutedWordsHash.contains(value)) {
        m_mutedWordsHash[value].value = value;
        m_mutedWordsHash[value].targets = targets;
    }
    QString key = removeSharp(value);
    if (m_mutedWordsTagHash.contains(key)) {
        m_mutedWordsTagHash[key].value = value;
        m_mutedWordsTagHash[key].targets = targets;
    }
}

void ConfigurableLabels::removeMutedWordItem(const int index)
{
    if (index < 0 || index >= mutedWordCount())
        return;
    QString value = m_mutedWords.at(index).value;
    if (m_mutedWordsHash.contains(value)) {
        m_mutedWordsHash.remove(value);
    }
    QString key = removeSharp(value);
    if (m_mutedWordsTagHash.contains(key)) {
        m_mutedWordsTagHash.remove(key);
    }
    m_mutedWords.removeAt(index);
}

void ConfigurableLabels::moveMutedWordItem(const int from, const int to)
{
    if (from < 0 || from >= mutedWordCount())
        return;
    if (to < 0 || to >= mutedWordCount())
        return;
    m_mutedWords.move(from, to);
}

int ConfigurableLabels::indexOfMutedWordItem(const QString &value) const
{
    for (int i = 0; i < m_mutedWords.count(); i++) {
        if (m_mutedWords.at(i).value == value) {
            return i;
        }
    }
    return -1;
}

bool ConfigurableLabels::containsMutedWords(const QString &text, const QStringList &tags,
                                            const bool partial_match) const
{
    if (partial_match) {
        // 主に日本語向けの部分一致
        for (const auto &word : m_mutedWords) {
            if (word.targets.contains(MutedWordTarget::Content)) {
                if (text.contains(word.value)) {
                    return true;
                }
            }
        }
    } else {
        // 単語が空白で区切られる言語向け
        QStringList words = text.split(m_regSpace);
        for (const auto &word : qAsConst(words)) {
            if (m_mutedWordsHash.contains(word)) {
                if (m_mutedWordsHash[word].targets.contains(MutedWordTarget::Content)) {
                    return true;
                }
            }
        }
    }
    // タグは共通
    for (const auto &tag : tags) {
        if (m_mutedWordsTagHash.contains(tag)) {
            if (m_mutedWordsTagHash[tag].targets.contains(MutedWordTarget::Tag)) {
                return true;
            }
        }
    }
    return false;
}

void ConfigurableLabels::clearMutedWord()
{
    m_mutedWords.clear();
    m_mutedWordsHash.clear();
    m_mutedWordsTagHash.clear();
}

bool ConfigurableLabels::enableAdultContent() const
{
    return m_enableAdultContent;
}

void ConfigurableLabels::setEnableAdultContent(bool newEnableAdultContent)
{
    m_enableAdultContent = newEnableAdultContent;
}

void ConfigurableLabels::initializeLabels()
{
    ConfigurableLabelItem item;

    // ラベルの情報
    // https://docs.bsky.app/docs/advanced-guides/moderation
    // https://atproto.com/specs/label
    // https://github.com/bluesky-social/atproto/blob/main/packages/api/definitions/labels.json
    // idはpreferenceの項目とのマッチングに使うのでconfigurable==trueの
    // もので重複させないこと

    // 個別のラベラーの設定は読み込みするときにする（スキップするときは消してはいけない）
    m_labels[GLOBAL_LABELER_KEY].clear();

    LabelerItem labeler_item;
    labeler_item.did = GLOBAL_LABELER_KEY;
    labeler_item.handle = "global_labeler";
    labeler_item.display_name = tr("Basic Moderation");
    labeler_item.description = tr("Basic configuration independent of moderation services.");
    m_labelers[GLOBAL_LABELER_KEY] = labeler_item;

    // global label
    item.values.clear();
    item.id = "system";
    item.title = tr("Content hidden");
    item.subtitle = tr("Moderator overrides for special cases.");
    item.warning = tr("Content hidden");
    item.values << "!hide";
    item.is_adult_imagery = false;
    item.foldable_range = ConfigurableLabelFoldableRange::Content;
    item.status = ConfigurableLabelStatus::Hide;
    item.configurable = false;
    m_labels[GLOBAL_LABELER_KEY].append(item);

    // global label
    item.values.clear();
    item.id = "system";
    item.title = tr("Content warning");
    item.subtitle = tr("Moderator overrides for special cases.");
    item.warning = tr("Content warning");
    item.values << "!warn";
    item.is_adult_imagery = false;
    item.foldable_range = ConfigurableLabelFoldableRange::Content;
    item.status = ConfigurableLabelStatus::Warning;
    item.configurable = false;
    m_labels[GLOBAL_LABELER_KEY].append(item);

    // global label
    item.values.clear();
    item.id = "porn";
    item.title = tr("Pornography");
    item.subtitle = tr("Explicit sexual images.");
    item.warning = tr("Sexually Explicit");
    item.values << "porn"
                << "nsfw";
    item.is_adult_imagery = true;
    item.foldable_range = ConfigurableLabelFoldableRange::Media;
    item.status = ConfigurableLabelStatus::Hide;
    item.configurable = true;
    m_labels[GLOBAL_LABELER_KEY].append(item);

    // global label
    item.values.clear();
    item.id = "sexual";
    item.title = tr("Sexually Suggestive");
    item.subtitle = tr("Does not include nudity");
    item.warning = tr("Sexually Suggestive");
    item.values << "sexual";
    item.is_adult_imagery = true;
    item.foldable_range = ConfigurableLabelFoldableRange::Media;
    item.status = ConfigurableLabelStatus::Warning;
    item.configurable = true;
    m_labels[GLOBAL_LABELER_KEY].append(item);

    // global label
    item.values.clear();
    item.id = "graphic-media";
    item.title = tr("Graphic Media");
    item.subtitle = tr("Explicit or potentially disturbing media.");
    item.warning = tr("Graphic Media");
    item.values << "graphic-media";
    item.is_adult_imagery = true;
    item.foldable_range = ConfigurableLabelFoldableRange::Media;
    item.status = ConfigurableLabelStatus::Warning;
    item.configurable = true;
    m_labels[GLOBAL_LABELER_KEY].append(item);

    // global label
    item.values.clear();
    item.id = "nudity";
    item.title = tr("Non-sexual Nudity");
    item.subtitle = tr("E.g. artistic nudes.");
    item.warning = tr("Non-sexual Nudity");
    item.values << "nudity";
    item.is_adult_imagery = false;
    item.foldable_range = ConfigurableLabelFoldableRange::Media;
    item.status = ConfigurableLabelStatus::Show;
    item.configurable = true;
    m_labels[GLOBAL_LABELER_KEY].append(item);

    // old label(compatibility)
    item.values.clear();
    item.id = "legal";
    item.title = tr("Legal");
    item.subtitle = tr("Content removed for legal reasons.");
    item.warning = tr("Legal");
    item.values << "dmca-violation"
                << "doxxing";
    item.is_adult_imagery = false;
    item.foldable_range = ConfigurableLabelFoldableRange::Content;
    item.status = ConfigurableLabelStatus::Hide;
    item.configurable = false;
    m_labels[GLOBAL_LABELER_KEY].append(item);

    // old label(compatibility)
    item.values.clear();
    item.id = "gore";
    item.title = tr("Violent / Bloody");
    item.subtitle = tr("Gore, self-harm, torture");
    item.warning = tr("Violence");
    item.values << "gore"
                << "self-harm"
                << "torture"
                << "nsfl"
                << "corpse";
    item.is_adult_imagery = true;
    item.foldable_range = ConfigurableLabelFoldableRange::Media;
    item.status = ConfigurableLabelStatus::Hide;
    item.configurable = false;
    m_labels[GLOBAL_LABELER_KEY].append(item);

    // old label(compatibility)
    item.values.clear();
    item.id = "hate";
    item.title = tr("Hate Group Iconography");
    item.subtitle = tr("Images of terror groups, articles covering events, etc.");
    item.warning = tr("Hate Groups");
    item.values << "icon-kkk"
                << "icon-nazi"
                << "icon-intolerant"
                << "behavior-intolerant"
                << "intolerant-race"
                << "intolerant-gender"
                << "intolerant-sexual-orientation"
                << "intolerant-religion"
                << "intolerant"
                << "threat";
    item.is_adult_imagery = false;
    item.foldable_range = ConfigurableLabelFoldableRange::Content;
    item.status = ConfigurableLabelStatus::Hide;
    item.configurable = false;
    m_labels[GLOBAL_LABELER_KEY].append(item);

    // old label(compatibility)
    item.values.clear();
    item.id = "spam";
    item.title = tr("Spam");
    item.subtitle = tr("Excessive unwanted interactions");
    item.warning = tr("Spam");
    item.values << "spam"
                << "spoiler";
    item.is_adult_imagery = false;
    item.foldable_range = ConfigurableLabelFoldableRange::Content;
    item.status = ConfigurableLabelStatus::Hide;
    item.configurable = false;
    m_labels[GLOBAL_LABELER_KEY].append(item);

    // old label(compatibility)
    item.values.clear();
    item.id = "impersonation";
    item.title = tr("Impersonation / Scam");
    item.subtitle = tr("Accounts falsely claiming to be people or orgs");
    item.warning = tr("Impersonation");
    item.values << "impersonation"
                << "account-security"
                << "net-abuse"
                << "scam";
    item.is_adult_imagery = false;
    item.foldable_range = ConfigurableLabelFoldableRange::Content;
    item.status = ConfigurableLabelStatus::Hide;
    item.configurable = false;
    m_labels[GLOBAL_LABELER_KEY].append(item);
}

bool ConfigurableLabels::putPreferences(const QString &json)
{
    AppBskyActorPutPreferences *pref = new AppBskyActorPutPreferences(this);
    connect(pref, &AppBskyActorPutPreferences::finished, [=](bool success) {
        if (success) {
            qDebug() << "finish put preferences.";
        }
        setRunning(false);
        emit finished(success);
        pref->deleteLater();
    });
    pref->setAccount(account());
    pref->putPreferences(json);
    return true;
}

QString ConfigurableLabels::updatePreferencesJson(const QString &src_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(src_json.toUtf8());
    QJsonObject root_object = json_doc.object();
    if (root_object.contains("preferences")) {
        QJsonValue preferences = root_object.value("preferences");
        QJsonArray dest_preferences;
        if (preferences.isArray()) {
            bool existAdult = false;
            QStringList existId;
            for (int i = 0; i < preferences.toArray().count(); i++) {
                if (!preferences.toArray().at(i).isObject())
                    continue;
                QJsonObject value = preferences.toArray().takeAt(i).toObject();
                if (value.value("$type")
                    == QStringLiteral("app.bsky.actor.defs#adultContentPref")) {
                    // ここで更新するデータはいったん消す
                } else if (value.value("$type")
                           == QStringLiteral("app.bsky.actor.defs#contentLabelPref")) {
                    // ここで更新するデータはいったん消す（順番を守るため）
                } else if (value.value("$type")
                                   == QStringLiteral("app.bsky.actor.defs#mutedWordsPref")
                           && !m_mutedWords.isEmpty()) {
                    // ここで更新するデータはいったん消す
                } else {
                    // その他のデータはそのまま引き継ぐ
                    dest_preferences.append(value);
                }
            }
            {
                QJsonObject value;
                value.insert("$type", QStringLiteral("app.bsky.actor.defs#adultContentPref"));
                value.insert("enabled", QJsonValue(enableAdultContent()));
                dest_preferences.append(value);
            }
            QMapIterator<QString, QList<ConfigurableLabelItem>> i(m_labels);
            while (i.hasNext()) {
                i.next();
                for (const auto &label : i.value()) {
                    if (!label.configurable || label.status == label.default_status)
                        continue;
                    QJsonObject value;
                    value.insert("$type", QStringLiteral("app.bsky.actor.defs#contentLabelPref"));
                    value.insert("label", QJsonValue(label.id));
                    if (!label.labeler_did.isEmpty()) {
                        value.insert("labelerDid", label.labeler_did);
                    }
                    if (label.status == ConfigurableLabelStatus::Hide) {
                        value.insert("visibility", QJsonValue("hide"));
                    } else if (label.status == ConfigurableLabelStatus::Warning) {
                        value.insert("visibility", QJsonValue("warn"));
                    } else if (label.status == ConfigurableLabelStatus::Show) {
                        value.insert("visibility", QJsonValue("ignore"));
                    }
                    dest_preferences.append(value);
                }
            }
            {
                QMap<int, QJsonArray> muted_items;
                for (const auto &muted_word : qAsConst(m_mutedWords)) {
                    QJsonObject item;
                    item.insert("value", muted_word.value);
                    QJsonArray targets;
                    if (muted_word.targets.contains(MutedWordTarget::Content)) {
                        targets.append(QJsonValue("content"));
                    }
                    if (muted_word.targets.contains(MutedWordTarget::Tag)) {
                        targets.append(QJsonValue("tag"));
                    }
                    item.insert("targets", targets);
                    muted_items[muted_word.group].append(item);
                }
                QMapIterator<int, QJsonArray> ii(muted_items);
                while (ii.hasNext()) {
                    ii.next();
                    QJsonObject value;
                    value.insert("$type", QStringLiteral("app.bsky.actor.defs#mutedWordsPref"));
                    value.insert("items", ii.value());
                    dest_preferences.append(value);
                }
            }
            root_object.insert("preferences", dest_preferences);
        }
    }
    json_doc.setObject(root_object);
#ifdef QT_DEBUG
    return json_doc.toJson();
#else
    return json_doc.toJson(QJsonDocument::Compact);
#endif
}

inline QString ConfigurableLabels::removeSharp(const QString &value) const
{
    return value.at(0) == "#" ? value.right(value.length() - 1) : value;
}

ConfigurableLabelFoldableRange ConfigurableLabels::toLabelFoldableRange(const QString &blurs) const
{
    ConfigurableLabelFoldableRange foldable_range = ConfigurableLabelFoldableRange::Content;
    if (blurs == "content") {
        foldable_range = ConfigurableLabelFoldableRange::Content;
    } else if (blurs == "media") {
        foldable_range = ConfigurableLabelFoldableRange::Media;
    } else if (blurs == "none") {
        foldable_range = ConfigurableLabelFoldableRange::None;
    }
    return foldable_range;
}

ConfigurableLabelStatus ConfigurableLabels::toLabelStatus(const QString &visibility) const
{
    ConfigurableLabelStatus status = ConfigurableLabelStatus::Hide;
    if (visibility == "show" || visibility == "ignore") {
        status = ConfigurableLabelStatus::Show;
    } else if (visibility == "warn") {
        status = ConfigurableLabelStatus::Warning;
    } else if (visibility == "hide") {
        status = ConfigurableLabelStatus::Hide;
    }
    return status;
}

ConfigurableLabelLevel ConfigurableLabels::toLabelLevel(const QString &severity) const
{
    ConfigurableLabelLevel level = ConfigurableLabelLevel::Alert;
    if (severity == "alert") {
        level = ConfigurableLabelLevel::Alert;
    } else if (severity == "inform") {
        level = ConfigurableLabelLevel::Inform;
    } else if (severity == "none") {
        level = ConfigurableLabelLevel::None;
    }
    return level;
}

bool ConfigurableLabels::refreshLabelers() const
{
    return m_refreshLabelers;
}

void ConfigurableLabels::setRefreshLabelers(bool newRefreshLabelers)
{
    m_refreshLabelers = newRefreshLabelers;
}

bool ConfigurableLabels::running() const
{
    return m_running;
}

void ConfigurableLabels::setRunning(bool newRunning)
{
    if (m_running == newRunning)
        return;
    m_running = newRunning;
    emit runningChanged(newRunning);
}
