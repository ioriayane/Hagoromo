#include "configurablelabels.h"

#include "atprotocol/app/bsky/actor/appbskyactorgetpreferences.h"
#include "atprotocol/app/bsky/actor/appbskyactorputpreferences.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

using AtProtocolInterface::AppBskyActorGetPreferences;
using AtProtocolInterface::AppBskyActorPutPreferences;

ConfigurableLabels::ConfigurableLabels(QObject *parent)
    : AtProtocolInterface::AtProtocolAccount { parent },
      m_enableAdultContent(true),
      m_running(false)
{
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

    AppBskyActorGetPreferences *pref = new AppBskyActorGetPreferences(this);
    connect(pref, &AppBskyActorGetPreferences::finished, [=](bool success) {
        if (success) {
            m_enableAdultContent = pref->adultContentPref().enabled;
            for (const auto &item : *pref->contentLabelPrefList()) {
                int index = indexOf(item.label);
                if (index >= 0) {
                    ConfigurableLabelStatus status = ConfigurableLabelStatus::Hide;
                    if (item.visibility == "show") {
                        status = ConfigurableLabelStatus::Show;
                    } else if (item.visibility == "warn") {
                        status = ConfigurableLabelStatus::Warning;
                    } else if (item.visibility == "hide") {
                        status = ConfigurableLabelStatus::Hide;
                    }
                    setStatus(index, status);
                }
            }
        }
        setRunning(false);
        emit finished(success);
        pref->deleteLater();
    });
    pref->setAccount(account());
    return pref->getPreferences();
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
    return pref->getPreferences();
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
    return pref->putPreferences(json);
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
                    value.insert("visibility", QJsonValue("show"));
                }
                dest_preferences.append(value);
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
