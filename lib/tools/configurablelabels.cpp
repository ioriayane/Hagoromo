#include "configurablelabels.h"

#include "atprotocol/app/bsky/actor/appbskyactorgetpreferences.h"
#include "atprotocol/app/bsky/actor/appbskyactorputpreferences.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QHash>

#define GLOBAL_LABELER_KEY QStringLiteral("__globally__")

using AtProtocolInterface::AppBskyActorGetPreferences;
using AtProtocolInterface::AppBskyActorPutPreferences;

ConfigurableLabels::ConfigurableLabels(QObject *parent)
    : AtProtocolInterface::AtProtocolAccount { parent },
      m_enableAdultContent(true),
      m_running(false)
{
    m_regSpace.setPattern("\\s");
    initializeLabels();
}

ConfigurableLabels &ConfigurableLabels::operator=(ConfigurableLabels &other)
{
    m_enableAdultContent = other.m_enableAdultContent;
    QMapIterator<QString, QList<ConfigurableLabelItem>> i(other.m_labels);
    while (i.hasNext()) {
        i.next();
        m_labels[i.key()].clear();
        for (const auto &label : i.value()) {
            m_labels[i.key()].append(label);
        }
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

    m_mutedWords.clear();
    m_mutedWordsHash.clear();
    m_mutedWordsTagHash.clear();

    AppBskyActorGetPreferences *pref = new AppBskyActorGetPreferences(this);
    connect(pref, &AppBskyActorGetPreferences::finished, [=](bool success) {
        if (success) {
            m_enableAdultContent = pref->adultContentPref().enabled;
            for (const auto &item : *pref->contentLabelPrefList()) {
                int index = indexOf(item.label, item.labelerDid);
                ConfigurableLabelStatus status = toLabelStatus(item.visibility);
                if (index >= 0) {
                    // labelerDid.isEmptyのときもこっち
                    setStatus(index, status, item.labelerDid);
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
        emit finished(success);
        pref->deleteLater();
    });
    pref->setAccount(account());
    pref->getPreferences();
    return true;
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

    QString key = labeler_did.isEmpty() ? GLOBAL_LABELER_KEY : labeler_did;
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
    if (!m_labels.contains(key))
        return ConfigurableLabelStatus::Show;

    ConfigurableLabelStatus result = ConfigurableLabelStatus::Show;
    for (const auto &item : m_labels.value(key)) {
        if (item.values.contains(label) && item.is_adult_imagery == for_image) {
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
    return result;
}

QString ConfigurableLabels::message(const QString &label, const bool for_image,
                                    const QString &labeler_did) const
{
    QString key = labeler_did.isEmpty() ? GLOBAL_LABELER_KEY : labeler_did;
    if (!m_labels.contains(key))
        return QString();

    QString result;
    for (const auto &item : m_labels.value(key)) {
        if (item.values.contains(label) && item.is_adult_imagery == for_image) {
            result = item.warning;
            break;
        }
    }
    return result;
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

    QString result;
    for (const auto &item : m_labels.value(key)) {
        if (item.values.contains(label) && item.is_adult_imagery == for_image) {
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
    // https://github.com/bluesky-social/atproto/blob/main/packages/api/docs/labels.md
    // idはpreferenceの項目とのマッチングに使うのでconfigurable==trueの
    // もので重複させないこと

    m_labels[GLOBAL_LABELER_KEY].clear();

    item.values.clear();
    item.id = "system";
    item.title = tr("Content hidden");
    item.subtitle = tr("Moderator overrides for special cases.");
    item.warning = tr("Content hidden");
    item.values << "!hide";
    item.is_adult_imagery = false;
    item.status = ConfigurableLabelStatus::Hide;
    item.configurable = false;
    m_labels[GLOBAL_LABELER_KEY].append(item);

    item.values.clear();
    item.id = "system";
    item.title = tr("Content warning");
    item.subtitle = tr("Moderator overrides for special cases.");
    item.warning = tr("Content warning");
    item.values << "!warn";
    item.is_adult_imagery = false;
    item.status = ConfigurableLabelStatus::Warning;
    item.configurable = false;
    m_labels[GLOBAL_LABELER_KEY].append(item);

    item.values.clear();
    item.id = "legal";
    item.title = tr("Legal");
    item.subtitle = tr("Content removed for legal reasons.");
    item.warning = tr("Legal");
    item.values << "dmca-violation"
                << "doxxing";
    item.is_adult_imagery = false;
    item.status = ConfigurableLabelStatus::Hide;
    item.configurable = false;
    m_labels[GLOBAL_LABELER_KEY].append(item);

    item.values.clear();
    item.id = "nsfw";
    item.title = tr("Explicit Sexual Images");
    item.subtitle = tr("i.e. pornography");
    item.warning = tr("Sexually Explicit");
    item.values << "porn"
                << "nsfw";
    item.is_adult_imagery = true;
    item.status = ConfigurableLabelStatus::Hide;
    item.configurable = true;
    m_labels[GLOBAL_LABELER_KEY].append(item);

    item.values.clear();
    item.id = "nudity";
    item.title = tr("Other Nudity");
    item.subtitle = tr("Including non-sexual and artistic");
    item.warning = tr("Nudity");
    item.values << "nudity";
    item.is_adult_imagery = true;
    item.status = ConfigurableLabelStatus::Hide;
    item.configurable = true;
    m_labels[GLOBAL_LABELER_KEY].append(item);

    item.values.clear();
    item.id = "suggestive";
    item.title = tr("Sexually Suggestive");
    item.subtitle = tr("Does not include nudity");
    item.warning = tr("Sexually Suggestive");
    item.values << "sexual";
    item.is_adult_imagery = true;
    item.status = ConfigurableLabelStatus::Warning;
    item.configurable = true;
    m_labels[GLOBAL_LABELER_KEY].append(item);

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
    item.status = ConfigurableLabelStatus::Hide;
    item.configurable = true;
    m_labels[GLOBAL_LABELER_KEY].append(item);

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
    item.status = ConfigurableLabelStatus::Hide;
    item.configurable = true;
    m_labels[GLOBAL_LABELER_KEY].append(item);

    item.values.clear();
    item.id = "spam";
    item.title = tr("Spam");
    item.subtitle = tr("Excessive unwanted interactions");
    item.warning = tr("Spam");
    item.values << "spam"
                << "spoiler";
    item.is_adult_imagery = false;
    item.status = ConfigurableLabelStatus::Hide;
    item.configurable = true;
    m_labels[GLOBAL_LABELER_KEY].append(item);

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
    item.status = ConfigurableLabelStatus::Hide;
    item.configurable = true;
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
                                   == QStringLiteral("app.bsky.actor.defs#contentLabelPref")
                           && !value.contains("labelerDid")) {
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
                    if (!label.configurable)
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
                for (const auto &muted_word : m_mutedWords) {
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
