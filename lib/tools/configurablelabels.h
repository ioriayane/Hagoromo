#ifndef CONFIGURABLELABELS_H
#define CONFIGURABLELABELS_H

#include <QObject>

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

class ConfigurableLabels : public QObject
{
    Q_OBJECT
public:
    explicit ConfigurableLabels(QObject *parent = nullptr);

    int count() const;
    ConfigurableLabelStatus visibility(const QString &label, const bool for_image) const;
    QString message(const QString &label, const bool for_image) const;
    void setStatus(const int index, const ConfigurableLabelStatus status);

    bool enableAdultContent() const;
    void setEnableAdultContent(bool newEnableAdultContent);

signals:

private:
    QList<ConfigurableLabelItem> m_labels;
    bool m_enableAdultContent;

    void initializeLabels();
};

#endif // CONFIGURABLELABELS_H
