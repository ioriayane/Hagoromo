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

    Q_INVOKABLE void setAccount(const QString &service, const QString &did, const QString &handle,
                                const QString &email, const QString &accessJwt,
                                const QString &refreshJwt);
    Q_INVOKABLE void reportPost(const QString &uri, const QString &cid,
                                Reporter::ReportReason reason);
    Q_INVOKABLE void reportAccount(const QString &did, Reporter::ReportReason reason);

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
