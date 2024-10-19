#ifndef REPORTER_H
#define REPORTER_H

#include "atprotocol/accessatprotocol.h"
#include <QObject>

class Reporter : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
public:
    explicit Reporter(QObject *parent = nullptr);

    enum ReportReason {
        ReasonSpam,
        ReasonSexual,
        ReasonRude,
        ReasonViolation,
        ReasonOther,
        ReasonMisleading
    };
    Q_ENUM(ReportReason)

    AtProtocolInterface::AccountData account() const;
    Q_INVOKABLE void setAccount(const QString &uuid);
    Q_INVOKABLE void reportPost(const QString &uri, const QString &cid, const QString &text,
                                const QStringList &labelers, Reporter::ReportReason reason);
    Q_INVOKABLE void reportAccount(const QString &did, const QString &text,
                                   const QStringList &labelers, Reporter::ReportReason reason);
    Q_INVOKABLE void reportMessage(const QString &did, const QString &convo_id,
                                   const QString &message_id, const QString &text,
                                   Reporter::ReportReason reason);
    bool running() const;
    void setRunning(bool newRunning);

signals:
    void errorOccured(const QString &code, const QString &message);
    void finished(bool success);
    void runningChanged();

private:
    AtProtocolInterface::AccountData m_account;
    QHash<Reporter::ReportReason, QString> m_reasonHash;

    bool m_running;
};

#endif // REPORTER_H
