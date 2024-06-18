#ifndef LABELERPROVIDER_H
#define LABELERPROVIDER_H

#include "atprotocol/accessatprotocol.h"
#include "configurablelabels.h"

#include <QObject>

class LabelerConnector : public QObject
{
    Q_OBJECT
public:
    explicit LabelerConnector(QObject *parent) : QObject(parent) { }
    ~LabelerConnector() { }

signals:
    void finished(bool success);
};

class LabelerProvider : public QObject
{
    Q_OBJECT
    explicit LabelerProvider(QObject *parent = nullptr);
    ~LabelerProvider();

public:
    enum RefleshMode : int {
        None,
        RefleshAll, // internal only
        RefleshConfigOnly, // internal only
        RefleshAuto,
    };

    static LabelerProvider *getInstance();
    void setAccount(const AtProtocolInterface::AccountData &account);
    void update(const AtProtocolInterface::AccountData &account, LabelerConnector *connector,
                const RefleshMode mode);
    int refreshInterval(const AtProtocolInterface::AccountData &account) const;
    void setRefreshInterval(const AtProtocolInterface::AccountData &account,
                            qint64 newRefreshInterval);

    QStringList labelerDids(const AtProtocolInterface::AccountData &account) const;
    ConfigurableLabelStatus visibility(const AtProtocolInterface::AccountData &account,
                                       const QString &label, const bool for_image,
                                       const QString &labeler_did = QString()) const;
    QString message(const AtProtocolInterface::AccountData &account, const QString &label,
                    const bool for_image, const QString &labeler_did = QString()) const;
    bool containsMutedWords(const AtProtocolInterface::AccountData &account, const QString &text,
                            const QStringList &tags, const bool partial_match) const;

signals:

private:
    class Private;
    QHash<QString, Private *> d;
    Q_DISABLE_COPY_MOVE(LabelerProvider)

    QString getAccountKey(const AtProtocolInterface::AccountData &account) const;
};

#endif // LABELERPROVIDER_H
