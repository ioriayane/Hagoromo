#ifndef REPORTER_H
#define REPORTER_H

#include "atprotocol/accessatprotocol.h"
#include <QObject>

struct ReportReasonGroup
{
    ReportReasonGroup() { }
    ReportReasonGroup(const QString &name, const QString &description)
    {
        this->name = name;
        this->description = description;
    }
    QString name;
    QString description;
};

class Reporter : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
public:
    explicit Reporter(QObject *parent = nullptr);

    enum ReportReason {
        // old spec
        ReasonSpam,
        ReasonSexual,
        ReasonRude,
        ReasonViolation,
        ReasonOther,
        ReasonMisleading,
        // new spec
        ReasonAppeal,
        ReasonViolenceAnimal,
        ReasonViolenceThreats,
        ReasonViolenceGraphicContent,
        ReasonViolenceGlorification,
        ReasonViolenceExtremistContent,
        ReasonViolenceTrafficking,
        ReasonViolenceOther,
        ReasonSexualAbuseContent,
        ReasonSexualNCII,
        ReasonSexualDeepfake,
        ReasonSexualAnimal,
        ReasonSexualUnlabeled,
        ReasonSexualOther,
        ReasonChildSafetyCSAM,
        ReasonChildSafetyGroom,
        ReasonChildSafetyPrivacy,
        ReasonChildSafetyHarassment,
        ReasonChildSafetyOther,
        ReasonHarassmentTroll,
        ReasonHarassmentTargeted,
        ReasonHarassmentHateSpeech,
        ReasonHarassmentDoxxing,
        ReasonHarassmentOther,
        ReasonMisleadingBot,
        ReasonMisleadingImpersonation,
        ReasonMisleadingSpam,
        ReasonMisleadingScam,
        ReasonMisleadingElections,
        ReasonMisleadingOther,
        ReasonRuleSiteSecurity,
        ReasonRuleProhibitedSales,
        ReasonRuleBanEvasion,
        ReasonRuleOther,
        ReasonSelfHarmContent,
        ReasonSelfHarmED,
        ReasonSelfHarmStunts,
        ReasonSelfHarmSubstances,
        ReasonSelfHarmOther,
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
    Q_INVOKABLE QStringList getReasonGroupNameList() const;
    Q_INVOKABLE QString getReasonGroupDescription(const QString &key) const;
    Q_INVOKABLE QString getReasonGroupName(const QString &key) const;
    Q_INVOKABLE bool equalReasonGroupName(const QString &key, const QString &name) const;

    bool running() const;
    void setRunning(bool newRunning);

signals:
    void errorOccured(const QString &code, const QString &message);
    void finished(bool success);
    void runningChanged();

private:
    AtProtocolInterface::AccountData m_account;
    QHash<Reporter::ReportReason, QString> m_reasonHash;
    QStringList m_reasonGroupNameList;
    // QHash<QString, QString> m_reasonGroupHash; // QHash<group_name, description>
    QHash<QString, ReportReasonGroup> m_reasonGroupHash; // QHash<group_name, description>

    bool m_running;
};

#endif // REPORTER_H
