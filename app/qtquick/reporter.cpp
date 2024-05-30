#include "reporter.h"

#include "extension/com/atproto/moderation/comatprotomoderationcreatereportex.h"

using AtProtocolInterface::ComAtprotoModerationCreateReportEx;

Reporter::Reporter(QObject *parent) : QObject { parent }, m_running(false)
{
    m_reasonHash[ReasonSpam] = "reasonSpam";
    m_reasonHash[ReasonSexual] = "reasonSexual";
    m_reasonHash[ReasonRude] = "reasonRude";
    m_reasonHash[ReasonViolation] = "reasonViolation";
    m_reasonHash[ReasonOther] = "reasonOther";
    m_reasonHash[ReasonMisleading] = "reasonMisleading";
}

void Reporter::setAccount(const QString &service, const QString &did, const QString &handle,
                          const QString &email, const QString &accessJwt, const QString &refreshJwt)
{
    m_account.service = service;
    m_account.did = did;
    m_account.handle = handle;
    m_account.email = email;
    m_account.accessJwt = accessJwt;
    m_account.refreshJwt = refreshJwt;
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
    report->setAccount(m_account);
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
    report->setAccount(m_account);
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
    report->setAccount(m_account);
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
