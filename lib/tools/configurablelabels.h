#ifndef CONFIGURABLELABELS_H
#define CONFIGURABLELABELS_H

#include "atprotocol/accessatprotocol.h"

enum class ConfigurableLabelStatus : int {
    Hide,
    Warning,
    Show,
};

struct ConfigurableLabelItem
{
    QString id;
    QString title;
    QString subtitle;
    QString warning;
    QStringList values;
    bool is_adult_imagery = false;
    ConfigurableLabelStatus status = ConfigurableLabelStatus::Hide;
};

class ConfigurableLabels : public AtProtocolInterface::AccessAtProtocol
{
    Q_OBJECT
public:
    explicit ConfigurableLabels(QObject *parent = nullptr);

    ConfigurableLabels &operator=(ConfigurableLabels &other);

    int count() const;
    void load();
    void save() const;
    int indexOf(const QString &id) const;
    ConfigurableLabelStatus visibility(const QString &label, const bool for_image) const;
    QString message(const QString &label, const bool for_image) const;
    void setStatus(const int index, const ConfigurableLabelStatus status);

    bool enableAdultContent() const;
    void setEnableAdultContent(bool newEnableAdultContent);

signals:

protected:
    virtual void parseJson(bool success, const QString reply_json)
    {
        Q_UNUSED(success)
        Q_UNUSED(reply_json)
    }

private:
    QList<ConfigurableLabelItem> m_labels;
    bool m_enableAdultContent;

    void initializeLabels();
};

#endif // CONFIGURABLELABELS_H
