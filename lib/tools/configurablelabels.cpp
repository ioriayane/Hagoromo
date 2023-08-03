#include "configurablelabels.h"

#include "atprotocol/app/bsky/actor/appbskyactorgetpreferences.h"
#include "atprotocol/app/bsky/actor/appbskyactorputpreferences.h"

using AtProtocolInterface::AppBskyActorGetPreferences;
using AtProtocolInterface::AppBskyActorPutPreferences;

ConfigurableLabels::ConfigurableLabels(QObject *parent)
    : AtProtocolInterface::AccessAtProtocol { parent }, m_enableAdultContent(true), m_running(false)
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

void ConfigurableLabels::load()
{
    if (running())
        return;
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
    pref->getPreferences();
}

void ConfigurableLabels::save() const
{
    //
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

    m_labels[index].status = status;
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

    item.id = "nsfw";
    item.title = "Explicit Sexual Images";
    item.subtitle = "i.e. pornography";
    item.warning = "Sexually Explicit";
    item.values << "porn"
                << "nsfl"
                << "nsfw";
    item.is_adult_imagery = true;
    item.status = ConfigurableLabelStatus::Hide;
    m_labels.append(item);

    item.values.clear();
    item.id = tr("nudity");
    item.title = tr("Other Nudity");
    item.subtitle = tr("Including non-sexual and artistic");
    item.warning = tr("Nudity");
    item.values << "nudity";
    item.is_adult_imagery = true;
    item.status = ConfigurableLabelStatus::Hide;
    m_labels.append(item);

    item.values.clear();
    item.id = tr("suggestive");
    item.title = tr("Sexually Suggestive");
    item.subtitle = tr("Does not include nudity");
    item.warning = tr("Sexually Suggestive");
    item.values << "sexual";
    item.is_adult_imagery = true;
    item.status = ConfigurableLabelStatus::Warning;
    m_labels.append(item);

    item.values.clear();
    item.id = tr("gore");
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
    m_labels.append(item);

    item.values.clear();
    item.id = tr("hate");
    item.title = tr("Hate Group Iconography");
    item.subtitle = tr("Images of terror groups, articles covering events, etc.");
    item.warning = tr("Hate Groups");
    item.values << "icon-kkk"
                << "icon-nazi"
                << "icon-intolerant"
                << "behavior-intolerant";
    item.is_adult_imagery = false;
    item.status = ConfigurableLabelStatus::Hide;
    m_labels.append(item);

    item.values.clear();
    item.id = tr("spam");
    item.title = tr("Spam");
    item.subtitle = tr("Excessive unwanted interactions");
    item.warning = tr("Spam");
    item.values << "spam";
    item.is_adult_imagery = false;
    item.status = ConfigurableLabelStatus::Hide;
    m_labels.append(item);

    item.values.clear();
    item.id = tr("impersonation");
    item.title = tr("Impersonation");
    item.subtitle = tr("Accounts falsely claiming to be people or orgs");
    item.warning = tr("Impersonation");
    item.values << "impersonation";
    item.is_adult_imagery = false;
    item.status = ConfigurableLabelStatus::Hide;
    m_labels.append(item);
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
