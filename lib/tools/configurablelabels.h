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

    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)

public:
    explicit ConfigurableLabels(QObject *parent = nullptr);

    ConfigurableLabels &operator=(ConfigurableLabels &other);

    int count() const;
    void load();
    void save() const;
    int indexOf(const QString &id) const;
    ConfigurableLabelStatus visibility(const QString &label, const bool for_image) const;
    QString message(const QString &label, const bool for_image) const;
    QString title(const int index) const;
    QString description(const int index) const;
    ConfigurableLabelStatus status(const int index) const;
    void setStatus(const int index, const ConfigurableLabelStatus status);

    bool enableAdultContent() const;
    void setEnableAdultContent(bool newEnableAdultContent);
    bool running() const;
    void setRunning(bool newRunning);

signals:
    void runningChanged(bool running);

protected:
    virtual void parseJson(bool success, const QString reply_json)
    {
        Q_UNUSED(success)
        Q_UNUSED(reply_json)
    }

private:
    void initializeLabels();

    QList<ConfigurableLabelItem> m_labels;
    bool m_enableAdultContent;
    bool m_running;
};

#endif // CONFIGURABLELABELS_H
