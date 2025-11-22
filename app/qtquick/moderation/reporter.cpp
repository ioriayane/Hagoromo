#include "reporter.h"

#include "extension/com/atproto/moderation/comatprotomoderationcreatereportex.h"
#include "tools/accountmanager.h"

using AtProtocolInterface::ComAtprotoModerationCreateReportEx;

Reporter::Reporter(QObject *parent) : QObject { parent }, m_running(false)
{
    m_reasonHash[ReasonSpam] = "com.atproto.moderation.defs#reasonSpam";
    m_reasonHash[ReasonSexual] = "com.atproto.moderation.defs#reasonSexual";
    m_reasonHash[ReasonRude] = "com.atproto.moderation.defs#reasonRude";
    m_reasonHash[ReasonViolation] = "com.atproto.moderation.defs#reasonViolation";
    m_reasonHash[ReasonOther] = "com.atproto.moderation.defs#reasonOther";
    m_reasonHash[ReasonMisleading] = "com.atproto.moderation.defs#reasonMisleading";
    // new spec
    m_reasonHash[ReasonViolenceAnimal] = "tools.ozone.report.defs#reasonViolenceAnimal";
    m_reasonHash[ReasonViolenceThreats] = "tools.ozone.report.defs#reasonViolenceThreats";
    m_reasonHash[ReasonViolenceGraphicContent] =
            "tools.ozone.report.defs#reasonViolenceGraphicContent";
    m_reasonHash[ReasonViolenceGlorification] =
            "tools.ozone.report.defs#reasonViolenceGlorification";
    m_reasonHash[ReasonViolenceExtremistContent] =
            "tools.ozone.report.defs#reasonViolenceExtremistContent";
    m_reasonHash[ReasonViolenceTrafficking] = "tools.ozone.report.defs#reasonViolenceTrafficking";
    m_reasonHash[ReasonViolenceOther] = "tools.ozone.report.defs#reasonViolenceOther";
    m_reasonHash[ReasonSexualAbuseContent] = "tools.ozone.report.defs#reasonSexualAbuseContent";
    m_reasonHash[ReasonSexualNCII] = "tools.ozone.report.defs#reasonSexualNCII";
    m_reasonHash[ReasonSexualDeepfake] = "tools.ozone.report.defs#reasonSexualDeepfake";
    m_reasonHash[ReasonSexualAnimal] = "tools.ozone.report.defs#reasonSexualAnimal";
    m_reasonHash[ReasonSexualUnlabeled] = "tools.ozone.report.defs#reasonSexualUnlabeled";
    m_reasonHash[ReasonSexualOther] = "tools.ozone.report.defs#reasonSexualOther";
    m_reasonHash[ReasonChildSafetyCSAM] = "tools.ozone.report.defs#reasonChildSafetyCSAM";
    m_reasonHash[ReasonChildSafetyGroom] = "tools.ozone.report.defs#reasonChildSafetyGroom";
    m_reasonHash[ReasonChildSafetyPrivacy] = "tools.ozone.report.defs#reasonChildSafetyPrivacy";
    m_reasonHash[ReasonChildSafetyHarassment] =
            "tools.ozone.report.defs#reasonChildSafetyHarassment";
    m_reasonHash[ReasonChildSafetyOther] = "tools.ozone.report.defs#reasonChildSafetyOther";
    m_reasonHash[ReasonHarassmentTroll] = "tools.ozone.report.defs#reasonHarassmentTroll";
    m_reasonHash[ReasonHarassmentTargeted] = "tools.ozone.report.defs#reasonHarassmentTargeted";
    m_reasonHash[ReasonHarassmentHateSpeech] = "tools.ozone.report.defs#reasonHarassmentHateSpeech";
    m_reasonHash[ReasonHarassmentDoxxing] = "tools.ozone.report.defs#reasonHarassmentDoxxing";
    m_reasonHash[ReasonHarassmentOther] = "tools.ozone.report.defs#reasonHarassmentOther";
    m_reasonHash[ReasonMisleadingBot] = "tools.ozone.report.defs#reasonMisleadingBot";
    m_reasonHash[ReasonMisleadingImpersonation] =
            "tools.ozone.report.defs#reasonMisleadingImpersonation";
    m_reasonHash[ReasonMisleadingSpam] = "tools.ozone.report.defs#reasonMisleadingSpam";
    m_reasonHash[ReasonMisleadingScam] = "tools.ozone.report.defs#reasonMisleadingScam";
    m_reasonHash[ReasonMisleadingElections] = "tools.ozone.report.defs#reasonMisleadingElections";
    m_reasonHash[ReasonMisleadingOther] = "tools.ozone.report.defs#reasonMisleadingOther";
    m_reasonHash[ReasonRuleSiteSecurity] = "tools.ozone.report.defs#reasonRuleSiteSecurity";
    m_reasonHash[ReasonRuleProhibitedSales] = "tools.ozone.report.defs#reasonRuleProhibitedSales";
    m_reasonHash[ReasonRuleBanEvasion] = "tools.ozone.report.defs#reasonRuleBanEvasion";
    m_reasonHash[ReasonRuleOther] = "tools.ozone.report.defs#reasonRuleOther";
    m_reasonHash[ReasonSelfHarmContent] = "tools.ozone.report.defs#reasonSelfHarmContent";
    m_reasonHash[ReasonSelfHarmED] = "tools.ozone.report.defs#reasonSelfHarmED";
    m_reasonHash[ReasonSelfHarmStunts] = "tools.ozone.report.defs#reasonSelfHarmStunts";
    m_reasonHash[ReasonSelfHarmSubstances] = "tools.ozone.report.defs#reasonSelfHarmSubstances";
    m_reasonHash[ReasonSelfHarmOther] = "tools.ozone.report.defs#reasonSelfHarmOther";
}

AtProtocolInterface::AccountData Reporter::account() const
{
    return AccountManager::getInstance()->getAccount(m_account.uuid);
}

void Reporter::setAccount(const QString &uuid)
{
    m_account.uuid = uuid;
}

void Reporter::reportPost(const QString &uri, const QString &cid, const QString &text,
                          const QStringList &labelers, Reporter::ReportReason reason)
{
    qDebug() << "reportPost" << uri << cid << reason << text;
    if (uri.isEmpty() || cid.isEmpty() || !m_reasonHash.contains(reason))
        return;
    if (running())
        return;
    setRunning(true);

    ComAtprotoModerationCreateReportEx *report = new ComAtprotoModerationCreateReportEx(this);
    connect(report, &ComAtprotoModerationCreateReportEx::finished, [=](bool success) {
        setRunning(false);
        emit errorOccured(report->errorCode(), report->errorMessage());
        emit finished(success);
        report->deleteLater();
    });
    report->setAccount(account());
    if (!labelers.isEmpty() && !labelers.first().isEmpty()) {
        report->appendRawHeader("atproto-proxy", labelers.first() + "#atproto_labeler");
    }
    report->reportPost(uri, cid, text, m_reasonHash[reason]);
}

void Reporter::reportAccount(const QString &did, const QString &text, const QStringList &labelers,
                             ReportReason reason)
{
    qDebug() << "reportAccount" << did << reason;
    if (did.isEmpty() || !m_reasonHash.contains(reason))
        return;
    if (running())
        return;
    setRunning(true);

    ComAtprotoModerationCreateReportEx *report = new ComAtprotoModerationCreateReportEx(this);
    connect(report, &ComAtprotoModerationCreateReportEx::finished, [=](bool success) {
        setRunning(false);
        emit errorOccured(report->errorCode(), report->errorMessage());
        emit finished(success);
        report->deleteLater();
    });
    report->setAccount(account());
    if (!labelers.isEmpty() && !labelers.first().isEmpty()) {
        report->appendRawHeader("atproto-proxy", labelers.first() + "#atproto_labeler");
    }
    report->reportAccount(did, text, m_reasonHash[reason]);
}

void Reporter::reportMessage(const QString &did, const QString &convo_id, const QString &message_id,
                             const QString &text, ReportReason reason)
{
    qDebug() << "reportMessage" << did << reason;
    if (did.isEmpty() || convo_id.isEmpty() || message_id.isEmpty()
        || !m_reasonHash.contains(reason))
        return;
    if (running())
        return;
    setRunning(true);

    ComAtprotoModerationCreateReportEx *report = new ComAtprotoModerationCreateReportEx(this);
    connect(report, &ComAtprotoModerationCreateReportEx::finished, [=](bool success) {
        setRunning(false);
        emit errorOccured(report->errorCode(), report->errorMessage());
        emit finished(success);
        report->deleteLater();
    });
    report->setAccount(account());
    report->reportMessage(did, convo_id, message_id, text, m_reasonHash[reason]);
}

bool Reporter::running() const
{
    return m_running;
}

void Reporter::setRunning(bool newRunning)
{
    if (m_running == newRunning)
        return;
    m_running = newRunning;
    emit runningChanged();
}
