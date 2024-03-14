#include "configurablelabels.h"

#include "atprotocol/app/bsky/actor/appbskyactorgetpreferences.h"
#include "atprotocol/app/bsky/actor/appbskyactorputpreferences.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QHash>

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
    if (m_labels.count() == other.m_labels.count()) {
        for (int i = 0; i < m_labels.count(); i++) {
            m_labels[i].status = other.m_labels.at(i).status;
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

int ConfigurableLabels::count() const
{
    return m_labels.count();
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
                int index = indexOf(item.label);
                if (index >= 0) {
                    ConfigurableLabelStatus status = ConfigurableLabelStatus::Hide;
                    if (item.visibility == "show" || item.visibility == "ignore") {
                        status = ConfigurableLabelStatus::Show;
                    } else if (item.visibility == "warn") {
                        status = ConfigurableLabelStatus::Warning;
                    } else if (item.visibility == "hide") {
                        status = ConfigurableLabelStatus::Hide;
                    }
                    setStatus(index, status);
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

int ConfigurableLabels::indexOf(const QString &id) const
{
    if (id.isEmpty())
        return -1;

    int index = 0;
    for (const auto &label : m_labels) {
        if (label.id == id)
            return index;
        index++;
    }
    return -1;
}

ConfigurableLabelStatus ConfigurableLabels::visibility(const QString &label,
                                                       const bool for_image) const
{
    ConfigurableLabelStatus result = ConfigurableLabelStatus::Show;
    for (const auto &item : m_labels) {
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

QString ConfigurableLabels::message(const QString &label, const bool for_image) const
{
    QString result;
    for (const auto &item : m_labels) {
        if (item.values.contains(label) && item.is_adult_imagery == for_image) {
            result = item.warning;
            break;
        }
    }
    return result;
}

QString ConfigurableLabels::title(const int index) const
{
    if (index < 0 || index >= m_labels.length())
        return QString();

    return m_labels.at(index).title;
}

QString ConfigurableLabels::title(const QString &label, const bool for_image) const
{
    QString result;
    for (const auto &item : m_labels) {
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

QString ConfigurableLabels::description(const int index) const
{
    if (index < 0 || index >= m_labels.length())
        return QString();
    return m_labels.at(index).subtitle;
}

ConfigurableLabelStatus ConfigurableLabels::status(const int index) const
{
    if (index < 0 || index >= m_labels.length())
        return ConfigurableLabelStatus::Show;

    return m_labels.at(index).status;
}

void ConfigurableLabels::setStatus(const int index, const ConfigurableLabelStatus status)
{
    if (index < 0 || index >= m_labels.length())
        return;
    if (m_labels[index].configurable == false)
        return;

    m_labels[index].status = status;
}

bool ConfigurableLabels::isAdultImagery(const int index) const
{
    if (index < 0 || index >= m_labels.length())
        return false;

    return m_labels.at(index).is_adult_imagery;
}

bool ConfigurableLabels::configurable(const int index) const
{
    if (index < 0 || index >= m_labels.length())
        return false;

    return m_labels.at(index).configurable;
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

    item.values.clear();
    item.id = "system";
    item.title = tr("Content hidden");
    item.subtitle = tr("Moderator overrides for special cases.");
    item.warning = tr("Content hidden");
    item.values << "!hide";
    item.is_adult_imagery = false;
    item.status = ConfigurableLabelStatus::Hide;
    item.configurable = false;
    m_labels.append(item);

    item.values.clear();
    item.id = "system";
    item.title = tr("Content warning");
    item.subtitle = tr("Moderator overrides for special cases.");
    item.warning = tr("Content warning");
    item.values << "!warn";
    item.is_adult_imagery = false;
    item.status = ConfigurableLabelStatus::Warning;
    item.configurable = false;
    m_labels.append(item);

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
    m_labels.append(item);

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
    m_labels.append(item);

    item.values.clear();
    item.id = "nudity";
    item.title = tr("Other Nudity");
    item.subtitle = tr("Including non-sexual and artistic");
    item.warning = tr("Nudity");
    item.values << "nudity";
    item.is_adult_imagery = true;
    item.status = ConfigurableLabelStatus::Hide;
    item.configurable = true;
    m_labels.append(item);

    item.values.clear();
    item.id = "suggestive";
    item.title = tr("Sexually Suggestive");
    item.subtitle = tr("Does not include nudity");
    item.warning = tr("Sexually Suggestive");
    item.values << "sexual";
    item.is_adult_imagery = true;
    item.status = ConfigurableLabelStatus::Warning;
    item.configurable = true;
    m_labels.append(item);

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
    m_labels.append(item);

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
    m_labels.append(item);

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
    m_labels.append(item);

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
    m_labels.append(item);
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
            for (const auto &label : qAsConst(m_labels)) {
                if (!label.configurable)
                    continue;
                QJsonObject value;
                value.insert("$type", QStringLiteral("app.bsky.actor.defs#contentLabelPref"));
                value.insert("label", QJsonValue(label.id));
                if (label.status == ConfigurableLabelStatus::Hide) {
                    value.insert("visibility", QJsonValue("hide"));
                } else if (label.status == ConfigurableLabelStatus::Warning) {
                    value.insert("visibility", QJsonValue("warn"));
                } else if (label.status == ConfigurableLabelStatus::Show) {
                    value.insert("visibility", QJsonValue("ignore"));
                }
                dest_preferences.append(value);
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
