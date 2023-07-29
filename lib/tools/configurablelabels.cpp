#include "configurablelabels.h"

ConfigurableLabels::ConfigurableLabels(QObject *parent)
    : QObject { parent }, m_enableAdultContent(true)
{
    initializeLabels();
}

int ConfigurableLabels::count() const
{
    return m_labels.count();
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
