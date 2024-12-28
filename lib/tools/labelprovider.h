#ifndef LABELPROVIDER_H
#define LABELPROVIDER_H

#include "atprotocol/accessatprotocol.h"

#include <QObject>
#include <QPointer>

struct LabelData
{
    QString name;
    QString avatar;
};

class LabelConnector : public QObject
{
    Q_OBJECT
public:
    explicit LabelConnector(QObject *parent) : QObject(parent) { }
    ~LabelConnector() { }

signals:
    void finished(const QString &labeler_did);
};

class LabelProvider : public QObject
{
    Q_OBJECT
    explicit LabelProvider(QObject *parent = nullptr);
    ~LabelProvider();

public:
    static LabelProvider *getInstance();

    void update(const QStringList labelers, const AtProtocolInterface::AccountData &account,
                LabelConnector *connector);
    LabelData getLabel(const QString &labeler_did, const QString &id) const;

    void appendConnector(const QString &did, LabelConnector *connector);
    void setLanguage(const QString &language);
    QLocale::Language language() const;
signals:

private:
    bool nowWorking(const QString &did);

    QHash<QString, AtProtocolType::AppBskyLabelerDefs::LabelerViewDetailed>
            m_lablers; // QHash<labeler dis, details>
    QHash<QString, QList<QPointer<LabelConnector>>> m_connectors; // QHash<laber did, connectors>
    QLocale::Language m_lang;
};

#endif // LABELPROVIDER_H
